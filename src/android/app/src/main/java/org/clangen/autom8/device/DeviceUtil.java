package org.clangen.autom8.device;

public final class DeviceUtil {
    public static boolean isDeviceToggleable(Device device) {
        final int type = device.getType();
        return type == DeviceType.APPLIANCE || type == DeviceType.LAMP;
    }
}
