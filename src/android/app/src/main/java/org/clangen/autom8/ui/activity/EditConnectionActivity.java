package org.clangen.autom8.ui.activity;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnFocusChangeListener;
import android.widget.EditText;

import androidx.appcompat.app.AppCompatActivity;

import org.clangen.autom8.R;
import org.clangen.autom8.connection.Connection;
import org.clangen.autom8.connection.ConnectionLibrary;
import org.clangen.autom8.service.ClientService;
import org.clangen.autom8.util.ToolbarUtil;

import java.security.MessageDigest;
import java.util.Formatter;

public class EditConnectionActivity extends AppCompatActivity {
    public static final int EDIT_CONNECTION_RESULT_OK = 1000;
    public static final int EDIT_CONNECTION_RESULT_CANCEL = 1001;
    public static final String EXTRA_DEFAULT_CONNECTION_ID = "org.clangen.autom8.EXTRA_DEFAULT_CONNECTION_ID";
    public static final String EXTRA_IS_FIRST_RUN = "org.clangen.autom8.EXTRA_IS_FIRST_RUN";

    private ViewHolder mViews = new ViewHolder();
    private Long mConnectionBeingEdited;
    private boolean mPasswordChanged = false;
    private boolean mDummyPasswordLoaded = false;

    private static boolean sFirstRunInfoDisplayed = false;

    private class ViewHolder {
        public EditText mAlias;
        public EditText mHost;
        public EditText mPort;
        public EditText mPassword;
        public View mOk;
        public View mCancel;
    }

    public static void start(Activity parentActivity) {
        Intent intent = new Intent();
        intent.setClass(parentActivity, EditConnectionActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        intent.setAction(Intent.ACTION_VIEW);

        parentActivity.startActivityForResult(intent, 0);
    }

    public static void start(Activity parentActivity, Long connectionId) {
        Intent intent = new Intent();
        intent.setClass(parentActivity, EditConnectionActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        intent.setAction(Intent.ACTION_VIEW);
        intent.putExtra(EXTRA_DEFAULT_CONNECTION_ID, connectionId);

        parentActivity.startActivityForResult(intent, 0);
    }

    public static void startFirstRun(Activity parentActivity) {
        Intent intent = new Intent();
        intent.setClass(parentActivity, EditConnectionActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        intent.setAction(Intent.ACTION_VIEW);
        intent.putExtra(EXTRA_IS_FIRST_RUN, true);

        parentActivity.startActivityForResult(intent, 0);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.edit_connection);

        init();
    }

    public void showFirstRunDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setPositiveButton(R.string.button_continue, null);
        builder.setTitle(R.string.dlg_welcome_title);
        builder.setMessage(R.string.dlg_welcome_desc);
        builder.show();
    }

    private void init() {
        ToolbarUtil.initSolid(this);

        final View mainView = findViewById(R.id.EditConnectionView);

        Long connectionId = null;

        Bundle extras = getIntent().getExtras();
        boolean firstRun = false;
        if (extras != null) {
            connectionId = (Long) extras.get(EXTRA_DEFAULT_CONNECTION_ID);
            firstRun = extras.getBoolean(EXTRA_IS_FIRST_RUN, false);
        }

        mConnectionBeingEdited = connectionId;
        setResult(EditConnectionActivity.EDIT_CONNECTION_RESULT_CANCEL);

        if (mConnectionBeingEdited == null) {
            setTitle(R.string.edit_create_connection);
        }

        mViews.mAlias = (EditText) mainView.findViewById(R.id.Name);
        mViews.mHost = (EditText) mainView.findViewById(R.id.Host);
        mViews.mPort = (EditText) mainView.findViewById(R.id.PortNumber);
        mViews.mPassword = (EditText) mainView.findViewById(R.id.Password);
        mViews.mOk = mainView.findViewById(R.id.OkButton);
        mViews.mCancel = mainView.findViewById(R.id.CancelButton);

        mViews.mOk.setOnClickListener(mOnOkClickListener);
        mViews.mCancel.setOnClickListener(mOnCancelClickListener);

        populateFromConnectionId(mConnectionBeingEdited);
        if ((firstRun) && ( ! sFirstRunInfoDisplayed)) {
            sFirstRunInfoDisplayed = true;
            showFirstRunDialog();
        }

        mPasswordChanged = false;
        mViews.mPassword.addTextChangedListener(mOnTextChangedListener);
        mViews.mPassword.setOnFocusChangeListener(mOnPasswordFocusChangedListener);
    }

    private OnFocusChangeListener mOnPasswordFocusChangedListener = new OnFocusChangeListener() {
        public void onFocusChange(View v, boolean hasFocus) {
            if (hasFocus && mDummyPasswordLoaded) {
                mViews.mPassword.setText("");
                mDummyPasswordLoaded = false;
            }
        }
    };

    private void populateFromConnectionId(Long connectionId) {
        if (connectionId == null) {
            return;
        }

        Connection connection =
            ConnectionLibrary.getInstance(this).getConnectionById(connectionId);

        if (connection != null) {
            mViews.mAlias.setText(connection.getAlias());
            mViews.mHost.setText(connection.getHost());
            mViews.mPort.setText(String.format("%s", connection.getPort()));

            if (connection.getPassword().length() > 0) {
                /*
                 * Set a dummy password because the hash is ridiculously long. When
                 * the user accepts the input, the field will only be hashed and saved
                 * if it changed. See getPassword().
                 */
                mDummyPasswordLoaded = true;
                mViews.mPassword.setText("0123456789");
            }
        }
    }

    private String getPassword() {
        if (mPasswordChanged) {
            return sha1(mViews.mPassword.getText().toString());
        }

        if (mConnectionBeingEdited != null) {
            Connection connection = ConnectionLibrary
                .getInstance(this)
                .getConnectionById(mConnectionBeingEdited);

            return connection.getPassword();
        }

        return "";
    }

    private void finish(int resultCode) {
        setResult(resultCode);
        finish();
    }

    private void showInvalidDataDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.dlg_invalid_connection_params_title);
        builder.setMessage(R.string.dlg_invalid_connection_params_desc);
        builder.setPositiveButton(R.string.button_ok, null);
        builder.show();
    }

    private static String sha1(String password) {
        byte[] hash;

        try {
            MessageDigest sha1 = MessageDigest.getInstance("SHA256");
            hash = sha1.digest(password.getBytes("UTF-8"));
        }
        catch (Exception ex) {
            throw new RuntimeException("SHA1 or UTF-8 not found.");
        }

        Formatter formatter = new Formatter();

        for (byte b : hash) {
            formatter.format("%02x", b);
        }

        return formatter.toString();
    }

    private OnClickListener mOnOkClickListener = new OnClickListener() {
        public void onClick(View v) {
            String name = mViews.mAlias.getText().toString();
            String host = mViews.mHost.getText().toString();
            String password = getPassword();
            int port;

            try {
                port = Integer.parseInt(mViews.mPort.getText().toString());
            }
            catch (NumberFormatException ne) {
                port = -1;
            }

            if ((name.length() > 0) && (host.length() > 0) && (port > 0 && port < 65535)) {
                ConnectionLibrary cm = ConnectionLibrary.getInstance(EditConnectionActivity.this);
                Connection connection = new Connection(name, host, port, password, 0, null);

                if (mConnectionBeingEdited != null) {
                    cm.update(mConnectionBeingEdited, connection);
                }
                else {
                    mConnectionBeingEdited = cm.add(connection);
                }

                /*
                 * If there's no default connection flag this new one as default.
                 */
                final Context context = EditConnectionActivity.this;
                Connection def = ConnectionLibrary.getDefaultConnection(context);
                if (def == null && mConnectionBeingEdited != -1) {
                    ConnectionLibrary.setDefaultConnection(context, mConnectionBeingEdited);
                    def = ConnectionLibrary.getDefaultConnection(context);
                }

                /*
                 * If we're editing (or just created) the default profile, we need
                 * to send a broadcast that it changed, so the ClientService can
                 * pick it up and act accordingly
                 */
                if (def != null && def.getDatabaseId().equals(mConnectionBeingEdited)) {
                    sendBroadcast(new Intent(ClientService.ACTION_DEFAULT_CONNECTION_CHANGED));
                }

                finish(EditConnectionActivity.EDIT_CONNECTION_RESULT_OK);
            }
            else {
                showInvalidDataDialog();
            }
        }
    };

    private OnClickListener mOnCancelClickListener = new OnClickListener() {
        public void onClick(View v) {
            finish(EditConnectionActivity.EDIT_CONNECTION_RESULT_CANCEL);
        }
    };

    private TextWatcher mOnTextChangedListener = new TextWatcher() {
        public void onTextChanged(CharSequence s, int start, int before, int count) {
            mPasswordChanged = true;
        }

        public void beforeTextChanged(CharSequence s, int start, int count, int after) {

        }

        public void afterTextChanged(Editable s) {
        }
    };
}
