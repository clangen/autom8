package org.clangen.autom8.device;

import android.content.Context;
import android.content.Intent;

import org.json.JSONObject;

import java.util.List;

public abstract class DeviceLibrary {
    public static final String ACTION_DEVICE_LIBRARY_REFRESHED = "org.clangen.autom8.ACTION_DEVICE_LIBRARY_REFRESHED";
    public static final String ACTION_DEVICE_UPDATED = "org.clangen.autom8.ACTION_DEVICE_UPDATED";
    public static final String ACTION_DEVICE_LIBRARY_CLEARED = "org.clangen.autom8.ACTION_DEVICE_LIBRARY_CLEARED";
    public static final String EXTRA_DEVICE_ADDRESS = "org.clangen.autom8.EXTRA_DEVICE_ADDRESS";

    public static final int MAXIMUM_DISPLAY_PRIORITY = 100;
    public static final int MINIMUM_DISPLAY_PRIORITY = 0;
    public static final int SECURITY_ALERT_DISPLAY_PRIORITY = MAXIMUM_DISPLAY_PRIORITY;
    public static final int SECURITY_SENSOR_DISPLAY_PRIORITY = MINIMUM_DISPLAY_PRIORITY;
    public static final int DEFAULT_DISPLAY_PRIORITY = 25;

    protected Context mContext;

    public DeviceLibrary(Context context) {
        mContext = context;
    }

    public abstract List<Device> getDeviceList();
    public abstract List<Group> getDeviceGroups();
    public abstract void setFromDeviceListJSON(JSONObject devices);
    public abstract void clear();
    public abstract int getAlertCount();
    public abstract int getDeviceCount();
    public abstract boolean update(JSONObject device);
    public abstract Device getDeviceByAddress(String str);

    protected synchronized void onReloaded() {
        mContext.sendBroadcast(new Intent(ACTION_DEVICE_LIBRARY_REFRESHED));
    }

    protected synchronized void onDeviceUpdated(final String address) {
        final Intent i = new Intent(ACTION_DEVICE_UPDATED);
        i.putExtra(EXTRA_DEVICE_ADDRESS, address);
        mContext.sendBroadcast(i);
    }

    protected synchronized void onCleared() {
        mContext.sendBroadcast(new Intent(ACTION_DEVICE_LIBRARY_CLEARED));
    }
}
