package org.clangen.autom8.net;

import android.content.Context;
import android.os.Handler;
import android.util.Log;

import org.clangen.autom8.connection.Connection;
import org.clangen.autom8.connection.ConnectionLibrary;

import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.math.BigInteger;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketException;
import java.security.KeyManagementException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.security.interfaces.RSAPublicKey;
import java.util.ArrayList;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.ReentrantLock;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

public class Client {
    private static final String TAG = "Client";

    private Handler mHandler = new Handler();
    private SSLSocket mSocket = null;
    private ReadThread mReadThread = null;
    private WriteThread mWriteThread = null;
    private Connection mConnection = null;
    private ConnectionLibrary mConnectionLibrary;
    private boolean mAuthenticated = false;
    private boolean mReconnectOnce = false;
    private OnMessageReceivedListener mOnResponseReceivedListener;
    private OnMessageReceivedListener mOnRequestReceivedListener;
    private OnStateChangedListener mOnStateChangedListener;
    private final Object mStateLock = new Object();
    private int mState = STATE_DISCONNECTED;

    public final static int STATE_CONNECTING = 0;
    public final static int STATE_CONNECTED = 1;
    public final static int STATE_AUTHENTICATING = 2;
    public final static int STATE_DISCONNECTED = 3;

    public final static int ERROR_NO_ERROR = 0;
    public final static int ERROR_TYPE_COULD_NOT_CONNECT = 1;
    public final static int ERROR_TYPE_CONNECT_DROPPED = 2;
    public final static int ERROR_TYPE_HANDSHAKE_FAILED = 3;
    public final static int ERROR_TYPE_AUTHENTICATION_FAILED = 4;

    private final static int ENQUEUE_BACK = 0;
    private final static int ENQUEUE_FRONT = 1;

    public interface OnStateChangedListener {
        void onStateChanged(int newState);
        void onError(int errorId);
    }

    public interface OnMessageReceivedListener {
        void onMessageReceived(Message message);
    }

    public Client(final Context context) {
        mAuthenticated = false;
        mState = STATE_DISCONNECTED;
        mConnectionLibrary = ConnectionLibrary.getInstance(context);
    }

    public void connect(Connection connection) {
        synchronized (mStateLock) {
            if (connection == null) {
                throw new NullPointerException("Cannot connect to a null connection!");
            }

            mConnection = connection;

            // if we're not disconnected then we'll need to disconnect before
            // reconnecting. the disconnection process happens asynchronously.
            if (mState != STATE_DISCONNECTED) {
                mReconnectOnce = true;
                disconnect();
            }
            else {
                mAuthenticated = false;
                changeState(STATE_CONNECTING);
                new ConnectThread().start();
            }
        }
    }

    public boolean isConnected() {
        return (mState == STATE_CONNECTED);
    }

    public boolean isDisconnected() {
        return (mState == STATE_DISCONNECTED);
    }

    public int getState() {
        return mState;
    }

    public Connection getConnection() {
        return mConnection;
    }

    public void disconnect() {
        if (mState == STATE_DISCONNECTED) {
            return;
        }

        synchronized (mStateLock) {
            if ((mSocket != null) || (mReadThread != null) || (mWriteThread != null)) {
                final Socket s = mSocket;
                final ReadThread r = mReadThread;
                final WriteThread w = mWriteThread;

                mSocket = null;
                mReadThread = null;
                mWriteThread = null;

                new Thread(new Runnable() { public void run() {
                    if (s != null) {
                        try {
                            s.close();
                        }
                        catch (Exception ex) {
                            Log.i(TAG, "Failed to close client socket: " + ex);
                        }

                        if (r != null) {
                            r.cancel();
                            r.interrupt();
                        }

                        if (w != null) {
                            w.cancel();
                            w.interrupt();
                        }

                        mHandler.post(onDisconnectCompletedRunnable);
                    }
                }}).start();
            }
        }
    }

    private Runnable onDisconnectCompletedRunnable = new Runnable() {
        public void run() {
            // if we get here all the cleanup has already been performed,
            // i.e. we're already disconnected!
            changeState(STATE_DISCONNECTED);

            if (mReconnectOnce) {
                mReconnectOnce = false;

                if (mConnection != null) {
                    connect(mConnection);
                }
            }
        }
    };

    public void sendMessage(Message message) {
        sendMessage(message, ENQUEUE_BACK);
    }

    public void sendPing() {
        sendMessage(new PingMessage(), ENQUEUE_FRONT);
    }

    public synchronized void setOnResponseReceivedListener(OnMessageReceivedListener listener) {
        mOnResponseReceivedListener = listener;
    }

    public synchronized void setOnRequestReceivedListener(OnMessageReceivedListener listener) {
        mOnRequestReceivedListener = listener;
    }

    public synchronized void setOnStateChangedListener(OnStateChangedListener listener) {
        mOnStateChangedListener = listener;
    }

    private void sendMessage(Message message, int order) {
        synchronized (mStateLock) {
            if (!mAuthenticated) {
                Log.e(TAG, "sendMessage: not authenticated, message was discarded!");
                return;
            }

            if (mWriteThread != null) {
                mWriteThread.enqueueMessage(message, order);
            }
        }
    }

    @SuppressWarnings("unused")
    private void onThreadFinished(Thread thread) {
        synchronized (mStateLock) {
            if (mState != STATE_DISCONNECTED) {
                disconnect();
            }
        }
    }

    private void changeState(int newState) {
        if (newState != getState()) {
            switch (newState) {
                case STATE_CONNECTING: Log.i(TAG, "connecting"); break;
                case STATE_CONNECTED: Log.i(TAG, "connected"); break;
                case STATE_AUTHENTICATING: Log.i(TAG, "authenticating"); break;
                case STATE_DISCONNECTED: Log.i(TAG, "disconnected"); break;
            }
        }

        boolean changed = false;
        synchronized (mStateLock) {
            if (newState != mState) {
                mState = newState;
                changed = true;
            }
        }

        // don't emit an event inside a critical section.
        if (changed) {
            onStateChanged(newState);
        }
    }

    private void onError(int newError) {
        switch (newError) {
        case ERROR_NO_ERROR: Log.i(TAG, "no error"); break;
        case ERROR_TYPE_COULD_NOT_CONNECT: Log.i(TAG, "could not connect"); break;
        case ERROR_TYPE_CONNECT_DROPPED: Log.i(TAG, "connection dropped"); break;
        case ERROR_TYPE_HANDSHAKE_FAILED: Log.i(TAG, "handshake failed"); break;
        case ERROR_TYPE_AUTHENTICATION_FAILED: Log.i(TAG, "authentication failed"); break;
        }

        OnStateChangedListener listener = mOnStateChangedListener;
        if (listener != null) {
            listener.onError(newError);
        }
    }

    private void onStateChanged(int newState) {
        OnStateChangedListener listener = mOnStateChangedListener;
        if (listener != null) {
            listener.onStateChanged(newState);
        }
    }

    private synchronized void handleAuthenticationResult(Message message) {
        // if we're not authenticated, the only message we can receive is
        // "authenticated." if we don't get this response we disconnect.
        if (message.getType() == MessageType.Response) {
            if (message.getName().equals("authenticated")) {
                mAuthenticated = true;
                changeState(STATE_CONNECTED);

                return;
            }
        }

        onError(ERROR_TYPE_AUTHENTICATION_FAILED);
        disconnect();
    }

    private void receiveMessage(ReadThread readThread, String base64Text) {
        synchronized (mStateLock) {
            if (readThread != mReadThread) {
                Log.i(TAG, "received message from non-active readThread. discarding");
                return;
            }

            final Message message = Message.create(base64Text);

            if (message != null) {
                synchronized (this) {
                    if (!mAuthenticated) {
                        handleAuthenticationResult(message);
                        return;
                    }
                }

                if (message.getType() == MessageType.Response) {
                    if (mOnResponseReceivedListener != null) {
                        mOnResponseReceivedListener.onMessageReceived(message);
                    }
                }
                else if (message.getType() == MessageType.Request) {
                    if (mOnRequestReceivedListener != null) {
                        mOnRequestReceivedListener.onMessageReceived(message);
                    }
                }
            }
        }
    }

    private TrustManager[] mTrustAllCertificates = new TrustManager[] {
        new X509TrustManager() {
            public void checkClientTrusted(X509Certificate[] chain, String authType) throws CertificateException {
            }

            public void checkServerTrusted(X509Certificate[] chain, String authType) throws CertificateException {
                RSAPublicKey publicKey = (RSAPublicKey) chain[0].getPublicKey();
                String publicKeyHex = publicKey.getModulus().toString(16).toUpperCase();

                try {
                    byte[] bytes = publicKeyHex.getBytes("ASCII");
                    MessageDigest digest = MessageDigest.getInstance("MD5");
                    digest.update(bytes, 0, bytes.length);
                    String md5str = new BigInteger(1, digest.digest()).toString(16);
                    md5str = (md5str.length() % 2 == 0) ? md5str : "0" + md5str;

                    StringBuilder sb = new StringBuilder();
                    char[] chars = md5str.toCharArray();
                    for (int i = 0; i < chars.length; i+=2) {
                        sb.append(chars[i]);
                        sb.append(chars[i+1]);
                        if (i < (chars.length - 2)) {
                            sb.append(':');
                        }
                    }

                    md5str = sb.toString().toLowerCase();

                    if (!mConnection.isVerified() || !md5str.equals(mConnection.getFingerprint())) {
                        mConnectionLibrary.markConnectionUnverified(mConnection.getDatabaseId(), md5str);
                        throw new CertificateException();
                    }
                }
                catch (Exception e) {
                    throw new CertificateException();
                }
            }

            public X509Certificate[] getAcceptedIssuers() {
                return null;
            }
        }
    };

    private class PingMessage extends Message {
        public String getBody() { return "{ }"; }
        public String getName() { return MessageName.Ping.toRawName(); }
        public MessageType getType() { return MessageType.Request; }
    }

    private class ConnectThread extends Thread {
        /*this*/ {
            setName("autom8 client connect thread");
        }

        public ConnectThread() {
        }

        public void run() {
            /* CAL 08/12/2014: really, really bad hack to fix problem with reconnect.
            TODO: figure out real race condition. */
            try { Thread.sleep(128); } catch(Exception ex) { }

            SSLSocket socket = null;

            // create the SSL connection, set the state to AUTHENTICATING
            try {
                synchronized (mStateLock) {
                    Connection connection = mConnection;

                    SSLContext context = SSLContext.getInstance("TLS");
                    context.init(null, mTrustAllCertificates, new SecureRandom());

                    socket = mSocket = (SSLSocket) context.getSocketFactory().createSocket();
                    socket.setUseClientMode(true);
                    socket.setWantClientAuth(false);
                    socket.connect(new InetSocketAddress(connection.getHost(), connection.getPort()), 5000);

                    changeState(STATE_AUTHENTICATING);
                }
            }
            catch (NoSuchAlgorithmException e) {
                Log.e(TAG, "ConnectThread.run: SSL context couldn't be created", e);
            }
            catch (KeyManagementException e) {
                Log.e(TAG, "ConnectThread.run: SSL context couldn't be initialized", e);
            }
            catch (IOException e) {
                Log.e(TAG, "ConnectThread.run: Connection failed", e);
            }

            // if we have a valid connection, start the handshake
            if (socket != null && Client.this.getState() == STATE_AUTHENTICATING) {
                try {
                    socket.startHandshake();
                    mWriteThread = new WriteThread();
                    mWriteThread.start(); // will send an AuthenticateMessage() upon startup
                    mReadThread = new ReadThread();
                    mReadThread.start();
                }
                catch (IOException ex) {
                    onError(ERROR_TYPE_HANDSHAKE_FAILED);
                    disconnect();
                }
            }
            else {
                onError(ERROR_TYPE_COULD_NOT_CONNECT);
                disconnect();
            }
        }
    }

    private class WriteThread extends Thread {
        private ReentrantLock mLock = new ReentrantLock();
        private Condition mMessageAvailable = mLock.newCondition();
        private ArrayList<Message> mMessageQueue = new ArrayList<>();
        private volatile boolean mShouldCancel = false;

        private class AuthenticateMessage extends Message {
            public String getBody() {
                return "{ \"password\": \"" + mConnection.getPassword() + "\" }";
            }

            public String getName() {
                return MessageName.Authenticate.toRawName();
            }

            public MessageType getType() {
                return MessageType.Request;
            }
        }

        public WriteThread() {
            setName("autom8 client write thread");
            enqueueMessage(new AuthenticateMessage());
        }

        public void enqueueMessage(Message message) {
            enqueueMessage(message, ENQUEUE_BACK);
        }

        public void enqueueMessage(Message message, int order) {
            mLock.lock();
            try {
                if (order == ENQUEUE_BACK) {
                    mMessageQueue.add(message);
                }
                else {
                    mMessageQueue.add(0, message);
                }

                mMessageAvailable.signalAll();
            }
            finally {
                mLock.unlock();
            }
        }

        public synchronized void cancel() {
            mShouldCancel = true;
        }

        private synchronized boolean shouldCancel() {
            return mShouldCancel;
        }

        public void run() {
            BufferedOutputStream outStream = null;

            try {
                Socket socket = mSocket;
                if (socket == null) {
                    return;
                }

                outStream = new BufferedOutputStream(socket.getOutputStream(), 2048);

                while (!shouldCancel()) {
                    try {
                        Message nextMessage = null;

                        // try to get the next message
                        try {
                            mLock.lock();
                            while (mMessageQueue.isEmpty()) {
                                mMessageAvailable.await();
                            }

                            nextMessage = mMessageQueue.remove(0);
                        }
                        finally {
                            mLock.unlock();
                        }

                        // hurrah! we got a new message
                        if (nextMessage != null) {
                            try {
                                outStream.write(nextMessage.toString().getBytes("UTF8"));
                                outStream.flush();
                            }
                            catch (UnsupportedEncodingException ex) {
                                Log.e("Client", "WriteThread.run: couldn't encode message", ex);
                            }
                            catch(IOException ex) {
                                Log.e("Client", "WriteThread.run: couldn't send message", ex);
                            }
                        }
                    }
                    catch (InterruptedException ex) {
                        // we're really disconnecting! rethrow so outer handler can do its work.
                        if (shouldCancel()) {
                            Log.i(TAG, "WriteThread.run: InterruptedException with (shouldCancel == true), rethrowing!");
                            throw ex;
                        }
                    }
                }
            }
            catch (IOException ex) {
                Log.e(TAG, "Client.WriteThread.run: unexpected IOException", ex);
            }
            catch (InterruptedException ex) {
                Log.i(TAG, "Client.WriteThread.run: InterruptedException");
            }
            finally {
                if (outStream != null) {
                    try {
                        outStream.close();
                    }
                    catch (IOException ex) {
                        // swallow. nothing we can do now.
                    }
                }

                onThreadFinished(this);
                Log.i(TAG, "Client.WriteThread.run: thread loop finished");
            }
        }
    }

    private class ReadThread extends Thread {
        private volatile boolean mShouldCancel = false;

        public ReadThread() {
            setName("autom8 client read thread");
        }

        public synchronized void cancel() {
            mShouldCancel = true;
        }

        private synchronized boolean shouldCancel() {
            return mShouldCancel;
        }

        public void run() {
            try {
                Socket socket = mSocket;
                if (socket == null) {
                    return;
                }

                boolean readFailed = false;
                ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
                InputStream in = socket.getInputStream();
                int currChar;
                while ((!shouldCancel()) && (!readFailed)) {
                    if ((currChar = in.read()) == -1) {
                        readFailed = true;
                    }
                    else {
                        if (currChar == '\0') {
                            String message = byteStream.toString("UTF-8");
                            byteStream = new ByteArrayOutputStream();
                            receiveMessage(this, message);
                        }
                        else {
                            byteStream.write(currChar);
                        }
                    }
                }
            }
            catch (SocketException se) {
                Log.e(TAG, "ReadThread.run: read failed, SocketException", se);
            }
            catch (IOException ioe) {
                Log.e(TAG, "ReadThread.run: read failed, IOException", ioe);
            }
            finally {
                Client.this.onThreadFinished(this);
                Log.i(TAG, "ReadThread.run: thread loop finished. shouldCancel: " + shouldCancel());
            }
        }
    }
}
