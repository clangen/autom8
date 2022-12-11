package org.clangen.autom8.device;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

public class Group implements Iterable<Device> {
    private static final DisplayPriorityComparator PRIORITY_COMPARATOR = new DisplayPriorityComparator();

    private ArrayList<Device> mDevices;
    private String mName;
    private int mToggleableCount = -1;

    public Group(String name, List<Device> devices) {
        mName = name;
        mDevices = new ArrayList<>(devices);
        Collections.sort(mDevices, PRIORITY_COMPARATOR);
    }

    public String getName() {
        return mName;
    }

    public int getAlertCount() {
        int count = 0;

        SecuritySensor sensor;
        for (Device device : mDevices) {
            if (device.getType() == DeviceType.SECURITY_SENSOR) {
                sensor = (SecuritySensor) device;
                count += (sensor.isArmed() && sensor.isTripped()) ? 1 : 0;
            }
        }
        return count;
    }

    public int getToggleableDeviceCount() {
        if (mToggleableCount == -1) {
            mToggleableCount = 0;

            for (Device device : mDevices) {
                if (DeviceUtil.isDeviceToggleable(device)) {
                    mToggleableCount++;
                }
            }
        }

        return mToggleableCount;
    }

    public boolean atLeastOneToggleableDeviceOn() {
        for (Device device : mDevices) {
            if (DeviceUtil.isDeviceToggleable(device) && device.getStatus() == DeviceStatus.ON) {
                return true;
            }
        }

        return false;
    }

    @Override
    public Iterator<Device> iterator() {
        return mDevices.iterator();
    }
}
