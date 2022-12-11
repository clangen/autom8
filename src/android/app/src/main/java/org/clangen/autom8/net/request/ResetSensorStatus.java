package org.clangen.autom8.net.request;

import java.util.HashMap;

import org.clangen.autom8.device.DeviceCommandType;
import org.clangen.autom8.device.SecuritySensor;

public class ResetSensorStatus extends SendDeviceCommand {
    private String mDeviceAddress;

    public ResetSensorStatus(SecuritySensor sensor) {
        mDeviceAddress = sensor.getAddress();
    }

    @Override
    protected String getCommandName() {
        return "reset_sensor_status";
    }

    @Override
    protected HashMap<String, String> getCommandParams() {
        return null;
    }

    @Override
    protected DeviceCommandType getCommandType() {
        return DeviceCommandType.PowerLine;
    }

    @Override
    protected String getDeviceAddress() {
        return mDeviceAddress;
    }
}
