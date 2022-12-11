package org.clangen.autom8.ui.model;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.Log;

import org.clangen.autom8.device.Device;
import org.clangen.autom8.device.DeviceLibrary;
import org.clangen.autom8.device.DeviceLibraryFactory;

import java.util.HashSet;

public abstract class BaseDeviceModel {
    private static final String TAG = "DeviceModelBase";

    private HashSet<String> mUpdatingSet;
    private DeviceLibrary mLibrary;
    private OnChangedListener mListener;
    private Context mContext;

    private static final IntentFilter LIBRARY_INTENTS;

    static {
        LIBRARY_INTENTS = new IntentFilter();
        LIBRARY_INTENTS.addAction(DeviceLibrary.ACTION_DEVICE_LIBRARY_CLEARED);
        LIBRARY_INTENTS.addAction(DeviceLibrary.ACTION_DEVICE_LIBRARY_REFRESHED);
        LIBRARY_INTENTS.addAction(DeviceLibrary.ACTION_DEVICE_UPDATED);
    }

    public interface OnChangedListener {
        void onChanged();
    }

    public BaseDeviceModel(Context context) {
        this(context, null);
    }

    public BaseDeviceModel(Context context, OnChangedListener listener) {
        mContext = context.getApplicationContext();
        mContext.registerReceiver(mLibraryIntentHandler, LIBRARY_INTENTS);
        mLibrary = DeviceLibraryFactory.getInstance(context);
        mListener = listener;
        mUpdatingSet = new HashSet<String>();
    }

    public abstract int size();
    public abstract Device get(int position);
    public abstract Device get(String address);

    public void setOnChangedListener(OnChangedListener listener) {
        mListener = listener;
    }

    public void close() {
        mContext.unregisterReceiver(mLibraryIntentHandler);
    }

    public void setUpdating(String address) {
        mUpdatingSet.add(address);
    }

    public boolean isUpdating(String address) {
        return mUpdatingSet.contains(address);
    }

    protected Context getContext() {
        return mContext;
    }

    protected DeviceLibrary getDeviceLibrary() {
        return mLibrary;
    }

    protected void onRequery() {
        /* for derived class use */
    }

    public void requery() {
        onRequery();
        mUpdatingSet.clear();

        if (mListener != null) {
            mListener.onChanged();
        }
    }

    private BroadcastReceiver mLibraryIntentHandler = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.i(TAG, "Handling event: " + intent.getAction());
            requery();
        }
    };
}
