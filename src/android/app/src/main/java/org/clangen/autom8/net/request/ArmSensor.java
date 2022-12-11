package org.clangen.autom8.net.request;

import java.util.HashMap;

import org.clangen.autom8.device.DeviceCommandType;
import org.clangen.autom8.device.SecuritySensor;

public class ArmSensor extends SendDeviceCommand {
    private String mDeviceAddress;
    private HashMap<String, String> mParams = new HashMap<String, String>();

    public ArmSensor(SecuritySensor sensor, boolean arm) {
        super();
        mDeviceAddress = sensor.getAddress();
        mParams.put("set_armed", arm ? "true" : "false");
    }

    @Override
    protected String getCommandName() {
        return "arm_sensor";
    }

    @Override
    protected HashMap<String, String> getCommandParams() {
        return mParams;
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
