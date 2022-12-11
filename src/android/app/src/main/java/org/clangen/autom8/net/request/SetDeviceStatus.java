package org.clangen.autom8.net.request;

import java.util.HashMap;

import org.clangen.autom8.device.Device;
import org.clangen.autom8.device.DeviceCommandType;

public class SetDeviceStatus extends SendDeviceCommand {
    private Device mDevice;
    private HashMap<String, String> mParams = new HashMap<>();

    public SetDeviceStatus(Device device, int newStatus) {
        super();
        mDevice = device;
        mParams.put("status", String.valueOf(newStatus));
    }

    @Override
    protected HashMap<String, String> getCommandParams() {
        return mParams;
    }

    @Override
    protected String getDeviceAddress() {
        return mDevice.getAddress();
    }

    @Override
    protected String getCommandName() {
        return "set_status";
    }

    @Override
    protected DeviceCommandType getCommandType() {
        return DeviceCommandType.PowerLine;
    }
}
