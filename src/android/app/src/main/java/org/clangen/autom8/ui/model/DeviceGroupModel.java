package org.clangen.autom8.ui.model;

import android.content.Context;

import org.clangen.autom8.device.Device;
import org.clangen.autom8.device.Group;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class DeviceGroupModel extends BaseDeviceModel {
    private ArrayList<Device> mFlattened = new ArrayList<Device>();
    private List<Group> mGroups = new ArrayList<Group>();
    private HashMap<Integer, Group> mDelimiters = new HashMap<Integer, Group>();

    public DeviceGroupModel(Context context) {
        super(context);
    }

    @Override
    protected void onRequery() {
        mGroups = getDeviceLibrary().getDeviceGroups();

        mFlattened.clear();
        mDelimiters.clear();
        for (Group group : mGroups) {
            mDelimiters.put(mFlattened.size(), group);

            for (Device device : group) {
                mFlattened.add(device);
            }
        }
    }

    @Override
    public int size() {
        return mFlattened.size();
    }

    @Override
    public Device get(int position) {
        return mFlattened.get(position);
    }

    public Group getDelimiter(int position) {
        return mDelimiters.get(position);
    }

    @Override
    public Device get(String address) {
        return getDeviceLibrary().getDeviceByAddress(address);
    }
}
