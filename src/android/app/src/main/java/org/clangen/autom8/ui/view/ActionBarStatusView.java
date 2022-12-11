package org.clangen.autom8.ui.view;

import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.TextView;

import org.clangen.autom8.Application;
import org.clangen.autom8.R;
import org.clangen.autom8.connection.Connection;
import org.clangen.autom8.connection.ConnectionLibrary;
import org.clangen.autom8.device.DeviceLibrary;
import org.clangen.autom8.device.DeviceLibraryFactory;
import org.clangen.autom8.net.Client;

public class ActionBarStatusView extends FrameLayout {
    private DeviceLibrary mDeviceLibrary;
    private View mConnectingView;
    private View mConnectedView;
    private View mDisconnectedView;
    private TextView mConnectedHostName;
    private TextView mConnectingTextView;
    private TextView mControllingTextView;

    public ActionBarStatusView(Context context) {
        super(context);
        init();
    }

    public ActionBarStatusView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public ActionBarStatusView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        init();
    }

     public void setClientServerState(int newState) {
        int progressState = View.GONE;
        int connectedState = View.GONE;
        int disconnectedState = View.GONE;
        int controllingState = View.GONE;
        String progressString = "";
        final Context context = getContext();

        switch (newState) {
            case Client.STATE_CONNECTING:
                progressState = View.VISIBLE;
                progressString = context.getString(R.string.status_connecting);
                mConnectedHostName.setText(getConnectedHostnameString());
                break;

            case Client.STATE_CONNECTED:
                connectedState = View.VISIBLE;
                controllingState = View.VISIBLE;
                refreshDeviceCount();
                mConnectedHostName.setText(getConnectedHostnameString());
                mConnectedView.setOnClickListener(null);
                break;

            case Client.STATE_AUTHENTICATING:
                progressState = View.VISIBLE;
                progressString = context.getString(R.string.status_authenticating);
                break;

            case Client.STATE_DISCONNECTED:
                disconnectedState = View.VISIBLE;
        }

        mConnectingView.setVisibility(progressState);
        mConnectingTextView.setText(progressString);
        mConnectedView.setVisibility(connectedState);
        mControllingTextView.setVisibility(controllingState);
        mDisconnectedView.setVisibility(disconnectedState);
    }

    public void refreshDeviceCount() {
        final int count = mDeviceLibrary.getDeviceCount();

        mControllingTextView.setText(
            getResources().getQuantityString(
                R.plurals.connected_device_count, count, count
            ));

        mControllingTextView.setVisibility(count == 0 ? View.GONE : View.VISIBLE);
    }

    private void init() {
        mDeviceLibrary = DeviceLibraryFactory.getInstance(getContext());

        LayoutInflater inflater  = (LayoutInflater)
            getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);

        inflater.inflate(R.layout.connection_status, this, true);
        mConnectingView = findViewById(R.id.ConnectingView);
        mConnectingTextView = (TextView) findViewById(R.id.ConnectingText);
        mConnectedView = findViewById(R.id.ConnectedView);
        mDisconnectedView = findViewById(R.id.DisconnectedView);
        mConnectedHostName = (TextView) findViewById(R.id.ConnectedHostNameView);
        mControllingTextView = (TextView) findViewById(R.id.ControllingDeviceCount);
    }

    private String getConnectedHostnameString() {
        Connection c = ConnectionLibrary.getDefaultConnection(getContext());

        if (c != null) {
            return String.format(" @ %s", c.getAlias());
        }

        return null;
    }

}