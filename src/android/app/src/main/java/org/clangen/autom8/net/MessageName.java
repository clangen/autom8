package org.clangen.autom8.net;

public enum MessageName {
    Authenticate("authenticate"),
    GetDeviceList("get_device_list"),
    DeviceStatusUpdating("device_status_updating"),
    DeviceStatusUpdated("device_status_updated"),
    SensorStatusChanged("sensor_status_changed"),
    SendDeviceCommand("send_device_command"),
    Ping("ping"),
    Pong("pong");

    private String mRawMessage;

    MessageName(final String rawMessage) {
        mRawMessage = rawMessage;
    }

    public String toRawName() {
        return mRawMessage;
    }

    public String toString() {
        return mRawMessage;
    }

    public boolean is(String rawMessage) {
        return mRawMessage.equals(rawMessage);
    }
}
