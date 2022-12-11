package org.clangen.autom8.ui.adapter;

import android.content.Context;

import org.clangen.autom8.ui.model.DeviceListModel;

public class DeviceListModelAdapter extends BaseDeviceModelAdapter {
    public DeviceListModelAdapter(Context context) {
        super(context);
        setModel(new DeviceListModel(context));
    }
}
