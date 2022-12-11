package org.clangen.autom8.ui.activity;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.WindowManager;

import androidx.appcompat.app.AppCompatActivity;

import org.clangen.autom8.R;
import org.clangen.autom8.connection.Connection;
import org.clangen.autom8.connection.ConnectionLibrary;
import org.clangen.autom8.device.DeviceLibrary;
import org.clangen.autom8.net.Client;
import org.clangen.autom8.service.ClientService;
import org.clangen.autom8.service.IClientService;
import org.clangen.autom8.ui.fragment.DevicesPagerFragment;
import org.clangen.autom8.ui.fragment.SettingsFragment;
import org.clangen.autom8.ui.view.ActionBarStatusView;
import org.clangen.autom8.util.ActivityUtil;
import org.clangen.autom8.util.ToolbarUtil;

public class DevicesActivity extends AppCompatActivity implements ClientServiceProvider {
    private final static String TAG = "DevicesActivity";

    private final static int MENU_ID_EDIT_CONNECTION = 0;
    private final static int MENU_ID_SETTINGS = 1;
    private final static int MENU_ID_RECONNECT = 2;

    private final static IntentFilter INTENT_FILTER;
    private static boolean sFirstRunChecked;

    private ActionBarStatusView mStatusView;
    private DevicesPagerFragment mPagerFragment;
    private ConnectionLibrary mConnectionLibrary;
    private IClientService mClientService;
    private boolean mPaused = true, mDestroyed;
    private boolean mServiceDisconnected;

    static {
        INTENT_FILTER = new IntentFilter();
        INTENT_FILTER.addAction(ClientService.ACTION_AUTHENTICATION_FAILED);
        INTENT_FILTER.addAction(ClientService.ACTION_CONNECTION_STATE_CHANGED);
        INTENT_FILTER.addAction(SettingsFragment.ACTION_TRANSLUCENCY_TOGGLED);
        INTENT_FILTER.addAction(DeviceLibrary.ACTION_DEVICE_LIBRARY_REFRESHED);
    }

    public static void startAndClearActivityStack(final Activity parent) {
        final Intent intent = new Intent(parent, DevicesActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        parent.startActivity(intent);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        checkEnableTranslucency();
        getIntent().addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);

        super.onCreate(savedInstanceState);

        bindService();

        setContentView(R.layout.devices_activity);
        initToolbar();

        registerReceiver(mBroadcastReceiver, INTENT_FILTER);

        mConnectionLibrary = ConnectionLibrary.getInstance(this);

        mPagerFragment = (DevicesPagerFragment)
            getSupportFragmentManager().findFragmentByTag(DevicesPagerFragment.TAG);

        setUiState(Client.STATE_DISCONNECTED);
    }

    @Override
    public void onResume() {
        super.onResume();
        mPaused = false;

        final Intent intent = new Intent(this, ClientService.class);
        intent.setAction(ClientService.ACTION_INC_CLIENT_COUNT);
        startService(intent);

        updateUiFromClientServiceState();
        checkFirstRun();

        if (mServiceDisconnected) {
            bindService();
        }
    }

    @Override
    public void onPause() {
        final Intent intent = new Intent(this, ClientService.class);
        intent.setAction(ClientService.ACTION_DEC_CLIENT_COUNT);
        startService(intent);

        mPaused = true;
        super.onPause();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == VerifyConnectionActivity.REQUEST_CODE) {
            if (resultCode == RESULT_CANCELED) {
                ConnectionManagerActivity.start(this);
                finish();
            }
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, MENU_ID_SETTINGS, 0, R.string.menu_settings);
        menu.add(0, MENU_ID_EDIT_CONNECTION, 1, R.string.menu_connection);
        menu.add(0, MENU_ID_RECONNECT, 2, R.string.menu_reconnect);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case MENU_ID_EDIT_CONNECTION:
                final int count = ConnectionLibrary.getInstance(this).count();
                if (count > 1) {
                    ConnectionManagerActivity.start(this);
                }
                else {
                    Connection c = ConnectionLibrary.getDefaultConnection(this);
                    EditConnectionActivity.start(this, (c == null) ? -1 : c.getDatabaseId());
                }
                return true;

            case MENU_ID_SETTINGS:
                SettingsActivity.start(this);
                return true;

            case MENU_ID_RECONNECT:
                reconnectToServer();
                return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    public IClientService getClientService() {
        return mClientService;
    }

    public void reconnectOrSetupConnection() {
        try {
            if (mClientService != null) {
                if (mConnectionLibrary.count() > 0) {
                    mClientService.reconnect();
                }
                else {
                    EditConnectionActivity.start(DevicesActivity.this);
                }
            }
        }
        catch (RemoteException re) {
            Log.d(TAG, "onReconnectClicked", re);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mDestroyed = true;
        unregisterReceiver(mBroadcastReceiver);
        unbindService();
    }

    private void reconnectToServer() {
        if (mClientService != null) {
            try {
                mClientService.reconnect();
            }
            catch (RemoteException re) {
                Log.d(TAG, "reconnected failed with a RemoteException");
            }
        }
    }

    private void bindService() {
        Intent intent = new Intent(this, ClientService.class);
        intent.setAction(ClientService.ACTION_START_SERVICE);
        startService(intent);
        bindService(intent, mServiceConnection, 0);
    }

    private void unbindService() {
        if (mClientService == null) {
            return;
        }

        if (mServiceConnection != null) {
            try {
                unbindService(mServiceConnection);
            }
            catch (Exception e) {
                Log.d(TAG, "unbindService threw", e);
            }
        }

        mClientService = null;
    }

    private void setUiState(int newState) {
        mStatusView.setClientServerState(newState);
        mPagerFragment.setClientServerState(newState);

        /* if we disconnect and the most recent connection is not verified... try to verify */
        if (newState == Client.STATE_DISCONNECTED) {
            final Connection connection = ConnectionLibrary.getDefaultConnection(this);
            if (connection != null && !connection.isVerified() && connection.getFingerprint().length() > 0) {
                VerifyConnectionActivity.start(this);
            }
        }
    }

    private void onAuthenticationFailed() {
        if (mPaused) {
            return;
        }

        DialogInterface.OnClickListener yesClickListener = (dialog, which) -> {
            final long id = ConnectionLibrary.getDefaultConnection(this).getDatabaseId();
            EditConnectionActivity.start(DevicesActivity.this, id);
        };

        AlertDialog.Builder builder = new AlertDialog.Builder(DevicesActivity.this);
        builder.setPositiveButton(R.string.button_yes, yesClickListener);
        builder.setNegativeButton(R.string.button_no, null);
        builder.setTitle(R.string.dlg_incorrect_pw_title);
        builder.setMessage(R.string.dlg_incorrect_pw_desc);
        builder.show();
    }

    private void updateUiFromClientServiceState() {
        if (mClientService != null) {
            try {
                final int state = mClientService.getState();
                setUiState(state);

                if (state == Client.STATE_DISCONNECTED) {
                    mClientService.reconnect();
                }
            }
            catch (RemoteException re) {
                Log.d(TAG, "serviceConnection.onServiceConnected", re);
            }
        }
    }

    private void checkEnableTranslucency() {
        if (ActivityUtil.isTranslucencyEnabled(this)) {
            setTheme(R.style.DeviceActivityTranslucentTheme);

            final int flags = WindowManager.LayoutParams.FLAG_DIM_BEHIND;
            getWindow().setFlags(flags, flags);

            WindowManager.LayoutParams params = getWindow().getAttributes();
            params.dimAmount = 0.45f;
            params.alpha = 1.0f;
            getWindow().setAttributes(params);
        }
        else {
            setTheme(R.style.DefaultActivityTheme);
        }
    }

    private void initToolbar() {
        mStatusView = new ActionBarStatusView(this);

        if (ActivityUtil.isTranslucencyEnabled(this)) {
            ToolbarUtil.initTranslucent(this).addView(mStatusView);
        }
        else {
            ToolbarUtil.initSolid(this).addView(mStatusView);
        }
    }

    private void checkFirstRun() {
        if (!sFirstRunChecked) {
            if (mConnectionLibrary.count() <= 0) {
                EditConnectionActivity.startFirstRun(this);
                sFirstRunChecked = true;
            }
        }
    }

    private ServiceConnection mServiceConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName name, final IBinder service) {
            Log.i(TAG, "ClientService bound to Activity");

            mClientService = IClientService.Stub.asInterface(service);
            mServiceDisconnected = false;

            if (mDestroyed) {
                unbindService();
            }
            else {
                updateUiFromClientServiceState();
            }
        }

        public void onServiceDisconnected(ComponentName name) {
            Log.i(TAG, "ClientService unbound from Activity");
            mServiceDisconnected = true;
        }
    };

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();

            if (ClientService.ACTION_AUTHENTICATION_FAILED.equals(action)) {
                onAuthenticationFailed();
            }
            else if (ClientService.ACTION_CONNECTION_STATE_CHANGED.equals(action)) {
                final int state = intent.getIntExtra(
                    ClientService.EXTRA_CONNECTION_STATE,
                    Client.STATE_DISCONNECTED);

                setUiState(state);
            }
            else if (SettingsFragment.ACTION_TRANSLUCENCY_TOGGLED.equals(action)) {
                finish();
            }
            else if (DeviceLibrary.ACTION_DEVICE_LIBRARY_REFRESHED.equals(action)) {
                mStatusView.refreshDeviceCount();
            }
        }
    };
}