package org.clangen.autom8.device;

import android.content.Context;

import org.clangen.autom8.device.impl.json.InMemoryDeviceLibrary;

public class DeviceLibraryFactory {
    private static DeviceLibrary mInstance;

    public static synchronized DeviceLibrary getInstance(Context context) {
        if (mInstance == null) {
            mInstance = new InMemoryDeviceLibrary(context);
        }

        return mInstance;
    }
}
