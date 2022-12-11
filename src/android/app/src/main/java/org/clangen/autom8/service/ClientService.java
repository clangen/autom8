package org.clangen.autom8.service;

import android.app.AlarmManager;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.util.Log;

import org.clangen.autom8.R;
import org.clangen.autom8.connection.Connection;
import org.clangen.autom8.connection.ConnectionLibrary;
import org.clangen.autom8.device.DeviceLibrary;
import org.clangen.autom8.device.DeviceLibraryFactory;
import org.clangen.autom8.device.impl.json.JsonDevice;
import org.clangen.autom8.net.Client;
import org.clangen.autom8.net.Client.OnMessageReceivedListener;
import org.clangen.autom8.net.Message;
import org.clangen.autom8.net.MessageName;
import org.clangen.autom8.net.request.GetDeviceList;
import org.clangen.autom8.ui.activity.DevicesActivity;

public class ClientService extends Service {
    private static final String TAG = "ClientService";

    public static final String ACTION_START_SERVICE = "org.clangen.autom8.service.ACTION_START_SERVICE";
    public static final String ACTION_RELOAD_SETTINGS = "org.clangen.autom8.service.ACTION_RELOAD_SETTINGS";
    public static final String ACTION_INC_CLIENT_COUNT = "org.clangen.autom8.service.ACTION_INC_CLIENT_COUNT";
    public static final String ACTION_DEC_CLIENT_COUNT = "org.clangen.autom8.service.ACTION_DEC_CLIENT_COUNT";
    public static final String ACTION_DEFAULT_CONNECTION_CHANGED = "org.clangen.autom8.ACTION_DEFAULT_CONNECTION_CHANGED";
    public static final String ACTION_AUTHENTICATION_FAILED = "org.clangen.autom8.ACTION_AUTHENTICATION_FAILED";
    public static final String ACTION_CONNECTION_STATE_CHANGED = "org.clangen.autom8.ACTION_CONNECTION_STATE_CHANGED";
    public static final String EXTRA_CONNECTION_STATE = "org.clangen.autom8.ACTION_CONNECTION_STATE";

    private static final int DEFAULT_HEARTBEAT_INTERVAL = 60 * 1000 * 10;
    private static final int SECURITY_NOTIFICATION_ID = 0xbeefbeef;
    private static final int BACKGROUND_NOTIFICATION_ID = 0xcafedead;
    private static final String ACTION_KEEP_CONNECTION_ALIVE = "org.clangen.autom8.service.KEEP_CONNECTION_ALIVE";
    private static final int MESSAGE_DELAYED_STOP = 0;
    private static final int MESSAGE_DELAYED_DISCONNECT = 1;
    private static final int MESSAGE_CLIENT_STATE_CHANGED = 2;
    private static final int MESSAGE_CLIENT_ERROR = 3;

    private Connection mConnection = null;
    private int mStartId;
    private int mLastError = Client.ERROR_NO_ERROR;
    private int mHeartbeatInterval;
    private int mClientCount;
    private boolean mSecurityNotification;
    private boolean mSecurityNotificationVisible;
    private boolean mDestroyed;
    private boolean mHeartbeatScheduled;
    private Bitmap mLargeAppIcon, mLargeAlertIcon;

    private NotificationManager mNotificationManager;
    private DeviceLibrary mLibrary;

    private static Client sClient;
    private static ServiceWakeLock sConnectionWakeLock;
    private static ServiceWakeLock sSensorChangedWakeLock;

    @Override
    public void onStart(Intent intent, int startId) {
        super.onStart(intent, startId);

        mStartId = startId;
        mHandler.removeMessages(MESSAGE_DELAYED_STOP);
        mHandler.removeMessages(MESSAGE_DELAYED_DISCONNECT);

        if (intent == null) {
            intent = new Intent();
        }

        final String a = intent.getAction();
        if (ACTION_INC_CLIENT_COUNT.equals(a)) {
            updateClientCount(1);
        }
        else if (ACTION_DEC_CLIENT_COUNT.equals(a)) {
            updateClientCount(-1);
        }
        else {
            updateBackgroundNotification();
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();

        if (sClient == null) {
            sClient = new Client(this);
            sConnectionWakeLock = new ServiceWakeLock("connection");
            sSensorChangedWakeLock = new ServiceWakeLock("sensorChanged");
        }

        Log.i(TAG, "service created");

        initIcons();

        sClient.setOnResponseReceivedListener(mOnResponseReceived);
        sClient.setOnStateChangedListener(mOnClientStateChangeListener);

        mLibrary = DeviceLibraryFactory.getInstance(this);

        registerReceiver(
            mHeartbeatReceiver,
            new IntentFilter(ACTION_KEEP_CONNECTION_ALIVE));

        registerReceiver(
            mNetworkStateChangedReceiver,
            new IntentFilter(ConnectivityManager.CONNECTIVITY_ACTION));

        final IntentFilter settingsFilter = new IntentFilter();
        settingsFilter.addAction(ACTION_RELOAD_SETTINGS);
        settingsFilter.addAction(ACTION_DEFAULT_CONNECTION_CHANGED);
        registerReceiver(mSettingsChangedFilter, settingsFilter);

        mNotificationManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);

        reloadSettings();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        unregisterReceiver(mHeartbeatReceiver);
        unregisterReceiver(mNetworkStateChangedReceiver);
        unregisterReceiver(mSettingsChangedFilter);

        stopConnectionStateChecking();

        sConnectionWakeLock.release();

        mDestroyed = true;
        sClient.setOnRequestReceivedListener(null);
        sClient.setOnResponseReceivedListener(null);
        sClient.setOnStateChangedListener(null);
        disconnect();

        Log.i(TAG, "service destroyed");
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mClientServiceBinder;
    }

    private int getState() {
        return sClient.getState();
    }

    private void reconnect() {
        Connection defaultConnection =
            ConnectionLibrary.getDefaultConnection(this);

        if (defaultConnection != null) {
            mLastError = Client.ERROR_NO_ERROR;
            mConnection = defaultConnection;

            sConnectionWakeLock.acquire(this);
            sClient.connect(mConnection);

            checkStartConnectionStateChecking();
        }
        else {
            stopConnectionStateChecking();
        }
    }

    private boolean reconnectIfNotConnected() {
        if ((!mDestroyed) && (sClient.isDisconnected())) {
            reconnect();
            return true;
        }

        return false;
    }

    private boolean reconnectIfDefaultConnectionChanged() {
        Connection defaultConnection =
            ConnectionLibrary.getDefaultConnection(ClientService.this);

        final Connection currentConnection = mConnection;

        if (defaultConnection == null) {
            if (currentConnection != null) {
                disconnect();
                return true;
            }

            return false; // CAL: should be gone?
        }

        if (!defaultConnection.equals(currentConnection)) {
            reconnect();
            return true;
        }

        return false;
    }

    private void disconnect() {
        sClient.disconnect();
    }

    private void processMessage(Message message) {
        boolean releaseWakeLockKLUDGE = false;

        // process alerts
        try {
            if (MessageName.GetDeviceList.is(message.getName())) {
                mLibrary.setFromDeviceListJSON(message.bodyToJSON());

                // if the device is asleep, and is automatically reconnected, it will send
                // a GetDeviceListRequest to check for security results. release the
                releaseWakeLockKLUDGE = true;
            }
            else if (MessageName.Pong.is(message.getName())) {
                // if we are using aggressive keep alives then we will send a PING every
                // DEFAULT_PING_INTERVAL milliseconds and wait for a PONG response.
                releaseWakeLockKLUDGE = true;
            }
            else {
                boolean sensorChanged = MessageName.SensorStatusChanged.is(message.getName());
                boolean deviceUpdated = MessageName.DeviceStatusUpdated.is(message.getName());

                if (sensorChanged || deviceUpdated) {
                    JsonDevice device = new JsonDevice(message.bodyToJSON());

                    if (device.isValid()) {
                        mLibrary.update(message.bodyToJSON());
                    }
                }
            }

            updateSecurityNotification();
        }
        finally {
            if (releaseWakeLockKLUDGE) {
                sConnectionWakeLock.release();
            }
        }
    }

    private boolean sendPingIfConnected() {
        if ((!mDestroyed) && (sClient.isConnected())) {
            sClient.sendPing();
            return true;
        }
        else {
            return false;
        }
    }

    private void sendMessage(Message message) {
        if (sClient.isConnected()) {
            sClient.sendMessage(message);
        }
    }

    private void scheduleHeartbeat() {
        if (mSecurityNotification && !mHeartbeatScheduled) {
            Log.i(TAG, "heartbeat scheduled");

            mHeartbeatScheduled = true;

            int interval = (mHeartbeatInterval > 0)
                ? mHeartbeatInterval : DEFAULT_HEARTBEAT_INTERVAL;

            Intent intent = new Intent(ACTION_KEEP_CONNECTION_ALIVE);
            PendingIntent pendingIntent = PendingIntent.getBroadcast(
                this, 0, intent, PendingIntent.FLAG_IMMUTABLE);

            ((AlarmManager) getSystemService(ALARM_SERVICE)).set(
                AlarmManager.RTC_WAKEUP,
                System.currentTimeMillis() + interval,
                pendingIntent);
        }
    }

    private void onStateChanged(int newState) {
        boolean releaseWakeLock = false;

        // release the wake lock if: we are disconnected (unconditionally), or if
        // we are connected and the UI is binded to us. if we're disconnected from
        // the UI then send a GetDeviceList() request to see if there are security alerts.
        // processMessage() will release the lock upon completion.
        try {
            switch (newState) {
            case Client.STATE_DISCONNECTED:
                releaseWakeLock = true;
                if (!mDestroyed) {
                    if (mLastError == Client.ERROR_TYPE_AUTHENTICATION_FAILED) {
                        mLibrary.clear();
                        sendBroadcast(new Intent(ACTION_AUTHENTICATION_FAILED));
                    }
                }
                break;

            case Client.STATE_CONNECTED:
                 sClient.sendMessage(new GetDeviceList());
                 if (mSecurityNotification) {
                     /*
                      * Will be released automatically by processMessage() when the next
                      * response comes in.
                      */
                     releaseWakeLock = false;
                 }
                 break;
            }
        }
        finally {
            if (releaseWakeLock) {
                sConnectionWakeLock.release();
            }
        }

        final Intent i = new Intent(ACTION_CONNECTION_STATE_CHANGED);
        i.putExtra(EXTRA_CONNECTION_STATE, newState);
        sendBroadcast(i);
    }

    private void checkStartConnectionStateChecking() { /* CAL: wtf this is really weird */
        if (mSecurityNotification) {
            scheduleHeartbeat();
        }
        else {
            stopConnectionStateChecking();
        }
    }

    private void stopConnectionStateChecking() {
        Intent intent = new Intent(ACTION_KEEP_CONNECTION_ALIVE);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(
            this, 0, intent, PendingIntent.FLAG_IMMUTABLE);
        ((AlarmManager) getSystemService(ALARM_SERVICE)).cancel(pendingIntent);
        mHeartbeatScheduled = false;
        Log.i(TAG, "heartbeat deactivated");
    }

    private void checkStopDelayed() {
        boolean stopDelayed = (!mSecurityNotification) && (mClientCount == 0);

        if (stopDelayed) {
            mHandler.sendMessageDelayed(mHandler.obtainMessage(MESSAGE_DELAYED_STOP), 10000);
            Log.i(TAG, "MESSAGE_DELAYED_STOP posted; no clients, no security notifications");
        }
    }

    private void updateClientCount(int delta) {
        mClientCount += delta;
        Log.i(TAG, "mClientCount changed to: " + mClientCount);

        if (mClientCount < 0) {
            Log.i(TAG, "mClientCount < 0, resetting to 0");
            mClientCount = 0;
        }

        updateBackgroundNotification();

        if (mClientCount == 0) {
            checkStopDelayed();
        }
        if (mClientCount >= 1) {
            mHandler.removeMessages(MESSAGE_DELAYED_DISCONNECT);
            mHandler.removeMessages(MESSAGE_DELAYED_STOP);

            Log.i(TAG, "invalidated any MESSAGE_DELAYED_STOP/DISCONNECT messages");

            reconnectIfNotConnected();
        }
    }

    private void updateBackgroundNotification() {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        boolean enabled = prefs.getBoolean(getString(R.string.pref_run_in_background), false);

        if (enabled && (mClientCount == 0)) {
            final Notification.Builder builder =
                new Notification.Builder(this)
                    .setContentTitle(getString(R.string.app_name))
                    .setContentText(getString(R.string.running_notification_desc))
                    .setSmallIcon(R.drawable.notification_monochrome)
                    .setLargeIcon(mLargeAppIcon);

            final Notification n = builder.getNotification();

            Intent intent = new Intent(this, DevicesActivity.class);
            intent.setAction(Intent.ACTION_VIEW);
            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);

            n.flags = Notification.FLAG_NO_CLEAR | Notification.FLAG_ONGOING_EVENT;
            n.defaults = 0;
            n.contentIntent = PendingIntent.getActivity(
                this, 0, intent, PendingIntent.FLAG_IMMUTABLE);

            mNotificationManager.notify(BACKGROUND_NOTIFICATION_ID, n);
        }
        else {
            mNotificationManager.cancel(BACKGROUND_NOTIFICATION_ID);
        }
    }

    private boolean updateSecurityNotification() {
        int alertCount = mLibrary.getAlertCount();
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        boolean enabled = prefs.getBoolean(getString(R.string.pref_security_notifications), false);

        if ((!enabled) || (alertCount == 0) || (!sClient.isConnected()) || (mClientCount > 0)) {
            mNotificationManager.cancel(SECURITY_NOTIFICATION_ID);
            mSecurityNotificationVisible = false;
            return false;
        }
        else {
            if (mSecurityNotificationVisible) {
                return false;
            }

            final String alertDetails = getResources().getQuantityString(
                R.plurals.security_notification_desc, alertCount, alertCount);

            final Notification.Builder builder =
                new Notification.Builder(this)
                    .setContentTitle(getString(R.string.app_name))
                    .setContentText(alertDetails)
                    .setSmallIcon(R.drawable.notification_monochrome)
                    .setLargeIcon(mLargeAlertIcon);

            if (android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                builder.setColor(Color.RED);
            }

            final Notification notification = builder.getNotification();

            Intent intent = new Intent();
            intent.setClassName(getPackageName(), DevicesActivity.class.getCanonicalName());
            intent.setAction(Intent.ACTION_VIEW);
            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);

            notification.flags = Notification.FLAG_NO_CLEAR | Notification.FLAG_AUTO_CANCEL | Notification.FLAG_SHOW_LIGHTS;
            notification.defaults = Notification.DEFAULT_ALL;
            notification.ledARGB = 0xffff0000;
            notification.ledOffMS = 1000;
            notification.ledOnMS = 1000;
            notification.contentIntent = PendingIntent.getActivity(
                this, 0, intent, PendingIntent.FLAG_IMMUTABLE);

            String sound = prefs.getString(getString(R.string.pref_notification_ringtone), null);
            if (sound != null && sound.length() > 0) {
                notification.sound = Uri.parse(sound);
                notification.defaults &= ~Notification.DEFAULT_SOUND;
            }

            mNotificationManager.notify(SECURITY_NOTIFICATION_ID, notification);
            mSecurityNotificationVisible = true;
            return true;
        }
    }

    private void reloadSettings() {
        SharedPreferences prefs =
            PreferenceManager.getDefaultSharedPreferences(ClientService.this);

        final boolean backgroundNotification = prefs.getBoolean(
            getString(R.string.pref_run_in_background), false);

        mSecurityNotification = backgroundNotification && prefs.getBoolean(
            getString(R.string.pref_security_notifications), false);

        try {
            double minutes = Double.parseDouble(prefs.getString(
                getString(R.string.pref_hearbeat_interval), "0.0"));

            mHeartbeatInterval = (int) (minutes * 60.0 * 1000.0);
        }
        catch (NumberFormatException ex) {
        }

        checkStartConnectionStateChecking();
        checkStopDelayed();
    }

    private void initIcons() {
        final Resources res = getResources();

        int nw = res.getDimensionPixelSize(android.R.dimen.notification_large_icon_width);
        int nh = res.getDimensionPixelSize(android.R.dimen.notification_large_icon_height);
        int size = Math.min(nh, nw) / 2;

        mLargeAppIcon = Bitmap.createScaledBitmap(
                BitmapFactory.decodeResource(res, R.drawable.icon), size, size, true);

        mLargeAlertIcon = Bitmap.createScaledBitmap(
                BitmapFactory.decodeResource(res, R.drawable.alert), size, size, true);
    }

    private Handler mHandler = new Handler() {
        public void handleMessage(android.os.Message msg) {
            switch (msg.what) {
            case MESSAGE_DELAYED_STOP:
                if ((!mSecurityNotification) && (mClientCount == 0)) {
                    Log.i(TAG, "MESSAGE_DELAYED_STOP: service shutting down...");
                    disconnect();
                    stopSelf(mStartId);
                    return;
                }

                Log.i(TAG, "MESSAGE_DELAYED_STOP: called but discarded");

                break;

            case MESSAGE_DELAYED_DISCONNECT:
                Log.i(TAG, "MESSAGE_DELAYED_DISCONNECT called");
                disconnect();
                break;

            case MESSAGE_CLIENT_STATE_CHANGED:
                onStateChanged(msg.arg1);
                break;

            case MESSAGE_CLIENT_ERROR:
                mLastError = msg.arg1;
                break;
            }
        }
    };

    private final IClientService.Stub mClientServiceBinder = new IClientService.Stub() {
        public void reconnect() throws RemoteException {
            ClientService.this.reconnect();
        }

        public void disconnect() throws RemoteException {
            ClientService.this.disconnect();
        }

        public void sendMessage(final Message message) throws RemoteException {
            ClientService.this.sendMessage(message);
        }

        public long getConnectionId() throws RemoteException {
            if (mConnection != null) {
                return mConnection.getDatabaseId();
            }

            return 0;
        }

        public int getState() throws RemoteException {
            return ClientService.this.getState();
        }
    };

    private Client.OnStateChangedListener mOnClientStateChangeListener = new Client.OnStateChangedListener() {
        public void onError(int errorId) {
            android.os.Message m = mHandler.obtainMessage(MESSAGE_CLIENT_ERROR);
            m.arg1 = errorId;
            mHandler.sendMessage(m);
        }

        public void onStateChanged(final int newState) {
            android.os.Message m = mHandler.obtainMessage(MESSAGE_CLIENT_STATE_CHANGED);
            m.arg1 = newState;
            mHandler.sendMessage(m);
        }
    };

    private OnMessageReceivedListener mOnResponseReceived = new OnMessageReceivedListener() {
        public void onMessageReceived(Message message) {
            try {
                /*
                 * wake the device up if a sensor status has changed, this will allow
                 * the notification to occur as soon as possible.
                 */
                if (MessageName.SensorStatusChanged.is(message.getName())) {
                    sSensorChangedWakeLock.acquire(ClientService.this);
                }

                processMessage(message);
            }
            finally {
                sSensorChangedWakeLock.release();
            }
        }
    };

    private BroadcastReceiver mHeartbeatReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            mHeartbeatScheduled = false;
            scheduleHeartbeat();

            sConnectionWakeLock.acquire(context);

            if (mSecurityNotification && sendPingIfConnected()) {
                // the ping has been queued successfully! do not release the wakelock until
                // the pong has been received! worst case scenerio: the connection is
                // severed before the pong is received, and we keep the wake lock acquired
                // until the next time the keepAlive is called.
                return;
            }

            if (!reconnectIfNotConnected()) {
                // if this call succeeded it means the client is disconnected, and we are
                // attempting to reconnect. wait until the client has reconnected to release
                // the wake lock.
                sConnectionWakeLock.release();
            }
        }
    };

    private BroadcastReceiver mNetworkStateChangedReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if ((mSecurityNotification || (mClientCount > 0))) {
                Log.i(TAG, "network connection changed");

                Bundle extras = intent.getExtras();
                NetworkInfo info = extras.getParcelable(WifiManager.EXTRA_NETWORK_INFO);

                if ((info != null) && (info.isConnected())) {
                    Log.i(TAG, "network up again, reconnecting...");

                    sConnectionWakeLock.acquire(context);
                    if (!reconnectIfNotConnected()) {
                        sConnectionWakeLock.release();
                    }
                }
            }
            else {
                Log.i(TAG, "network connection changed, but security notifications disabled" +
                      " and no clients connected. doing nothing.");
            }
        }
    };

    private BroadcastReceiver mSettingsChangedFilter = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();

            if (ACTION_RELOAD_SETTINGS.equals(action)) {
                Log.i(TAG, "settings change detected, reloading...");
                reloadSettings();
            }
            else if (ACTION_DEFAULT_CONNECTION_CHANGED.equals(action)) {
                Log.i(TAG, "default connection profile changed, reconnecting...");
                reconnectIfDefaultConnectionChanged();
            }
        }
    };
}
