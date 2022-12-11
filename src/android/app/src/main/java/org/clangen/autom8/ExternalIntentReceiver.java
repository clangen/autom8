package org.clangen.autom8;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

import org.clangen.autom8.connection.Connection;
import org.clangen.autom8.connection.ConnectionLibrary;
import org.clangen.autom8.service.ClientService;

import java.util.List;

public class ExternalIntentReceiver extends BroadcastReceiver {
    private static final String ACTION_CHANGE_CONNECTION = "org.clangen.autom8.CHANGE_CONNECTION";
    private static final String EXTRA_CONNECTION_NAME = "connection_name";

    @Override
    public void onReceive(Context context, Intent intent) {
        final String action = intent.getAction();
        if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
            onBootCompleted(context);
        }
        else if (ACTION_CHANGE_CONNECTION.equals(action)) {
            onChangeProfile(context, intent);
        }
    }

    private void onBootCompleted(final Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        if (prefs.getBoolean(context.getString(R.string.pref_start_at_boot), false)) {
            final Intent intent = new Intent(context, ClientService.class);
            intent.setAction(ClientService.ACTION_START_SERVICE);
            context.startService(intent);
        }
    }

    private void onChangeProfile(final Context context, final Intent intent) {
        String connectionName = intent.getStringExtra(EXTRA_CONNECTION_NAME);

        if (connectionName != null && !connectionName.isEmpty()) {
            connectionName = connectionName.trim().toLowerCase();

            final ConnectionLibrary connectionLibrary = ConnectionLibrary.getInstance(context);
            final List<Connection> connections = connectionLibrary.getConnections();

            for (final Connection c : connections) {
                final String currentName = c.getAlias().trim().toLowerCase();

                if (currentName.equals(connectionName)) {
                    ConnectionLibrary.setDefaultConnection(context, c.getDatabaseId());

                    final Intent startServiceIntent = new Intent(context, ClientService.class);
                    startServiceIntent.setAction(ClientService.ACTION_START_SERVICE);
                    context.startService(intent);

                    context.sendBroadcast(new Intent(ClientService.ACTION_DEFAULT_CONNECTION_CHANGED));
                    break;
                }
            }
        }
    }
}
