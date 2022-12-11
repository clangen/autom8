package org.clangen.autom8.ui.activity;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import org.clangen.autom8.R;
import org.clangen.autom8.connection.Connection;
import org.clangen.autom8.connection.ConnectionLibrary;
import org.clangen.autom8.service.ClientService;
import org.clangen.autom8.util.ToolbarUtil;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

public class ConnectionManagerActivity extends AppCompatActivity {
    private View mMainView;
    private ViewHolder mViews = new ViewHolder();
    private List<Connection> mConnections = new ArrayList<>();

    private static class ViewHolder {
        ListView mListView;
        View mAddConnectionView;
        View mCloseView;
        View mNoConnectionsView;
    }

    private static class ItemViewHolder {
        View mRow;
        TextView mName;
        TextView mDetails;
        View mDelete;
        View mEdit;
        Connection mConnection;
    }

    public static void start(Activity parentActivity) {
        Intent intent = new Intent();
        intent.setClass(parentActivity, ConnectionManagerActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        intent.setAction(Intent.ACTION_VIEW);
        parentActivity.startActivityForResult(intent, 0);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setResult(RESULT_CANCELED);

        setContentView(R.layout.connection_manager);
        mMainView = findViewById(R.id.ConnectionManagerView);

        ToolbarUtil.initSolid(this);

        initialize();
    }

    @Override
    protected void onResume() {
        super.onResume();
        onConnectionsUpdated();
    }

    private void onConnectionsUpdated() {
        mConnections = ConnectionLibrary.getInstance(this).getConnections();

        if (mConnections.size() > 0) {
            mViews.mNoConnectionsView.setVisibility(View.GONE);
            mViews.mListView.setVisibility(View.VISIBLE);
            mViews.mListView.setAdapter(mListAdapter);
            mListAdapter.notifyDataSetInvalidated();
        }
        else {
            mViews.mNoConnectionsView.setVisibility(View.VISIBLE);
            mViews.mListView.setVisibility(View.GONE);
        }
    }

    private void initialize() {
        mViews.mListView = (ListView) mMainView.findViewById(R.id.ListView);
        mViews.mListView.setItemsCanFocus(true);

        mViews.mNoConnectionsView = mMainView.findViewById(R.id.NoConnectionsView);
        mViews.mAddConnectionView = mMainView.findViewById(R.id.AddButton);
        mViews.mAddConnectionView.setOnClickListener(mOnAddConnectionClicked);
        mViews.mCloseView = mMainView.findViewById(R.id.CloseButton);
        mViews.mCloseView.setOnClickListener(mOnCloseClicked);
    }

    private void confirmAndDeleteConnection(final Connection connection) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.dlg_delete_connection_title);
        builder.setMessage(R.string.dlg_delete_connection_desc);

        builder.setPositiveButton(R.string.button_yes, (dialog, which) -> {
            if (ConnectionLibrary.getInstance(ConnectionManagerActivity.this).delete(connection)) {
                ConnectionManagerActivity.this.onConnectionsUpdated();
            }
        });

        builder.setNegativeButton(R.string.button_no, null);
        builder.show();
    }

    private OnClickListener mOnConnectionRowClicked = (view) -> {
        final Context context = ConnectionManagerActivity.this;
        final ItemViewHolder holder = (ItemViewHolder) view.getTag();
        final Connection connection = holder.mConnection;

        if ((connection != null) && (connection.getDatabaseId() != null)) {
            ConnectionLibrary.setDefaultConnection(context, connection.getDatabaseId());
            sendBroadcast(new Intent(ClientService.ACTION_DEFAULT_CONNECTION_CHANGED));

            DevicesActivity.startAndClearActivityStack(this);
            if (!connection.isVerified()) {
                overridePendingTransition(0, 0);
            }

            finish();
        }
    };

    private OnClickListener mOnCloseClicked =
        (view) -> DevicesActivity.startAndClearActivityStack(this);

    private OnClickListener mOnAddConnectionClicked = (view) ->
        EditConnectionActivity.start(ConnectionManagerActivity.this);

    private OnClickListener mOnEditConnectionClicked = (view) -> {
        final ItemViewHolder holder = (ItemViewHolder) view.getTag();

        EditConnectionActivity.start(
            ConnectionManagerActivity.this,
            holder.mConnection.getDatabaseId());
    };

    private OnClickListener mOnDeleteConnectionClicked = (view) -> {
        final ItemViewHolder holder = (ItemViewHolder) view.getTag();
        confirmAndDeleteConnection(holder.mConnection);
    };

    private BaseAdapter mListAdapter = new BaseAdapter() {
        public int getCount() {
            return mConnections.size();
        }

        public Object getItem(int position) {
            return mConnections.get(position);
        }

        public long getItemId(int position) {
            return position;
        }

        public View getView(int position, View convertView, ViewGroup parent) {
            if ((convertView == null) || (convertView.getId() != R.id.ConnectionItem)) {
                convertView = getLayoutInflater().inflate(
                    R.layout.connection_manager_item, parent, false);

                ItemViewHolder holder = new ItemViewHolder();
                holder.mRow = convertView;
                holder.mName = (TextView) convertView.findViewById(R.id.Name);
                holder.mDetails = (TextView) convertView.findViewById(R.id.Details);
                holder.mEdit = convertView.findViewById(R.id.EditButton);
                holder.mDelete = convertView.findViewById(R.id.DeleteButton);

                convertView.setTag(holder);
                holder.mDelete.setTag(holder);
                holder.mEdit.setTag(holder);
                holder.mRow.setTag(holder);
            }

            Connection connection = mConnections.get(position);
            ItemViewHolder holder = (ItemViewHolder) convertView.getTag();

            holder.mName.setText(connection.getAlias());
            holder.mDetails.setText(String.format(
                Locale.ENGLISH, "%s:%d", connection.getHost(), connection.getPort()));
            holder.mConnection = connection;
            holder.mDelete.setOnClickListener(mOnDeleteConnectionClicked);
            holder.mEdit.setOnClickListener(mOnEditConnectionClicked);
            holder.mRow.setOnClickListener(mOnConnectionRowClicked);

            return convertView;
        }
    };
}
