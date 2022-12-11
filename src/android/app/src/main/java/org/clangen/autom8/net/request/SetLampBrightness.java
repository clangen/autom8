package org.clangen.autom8.net.request;

import java.util.HashMap;

import org.clangen.autom8.device.DeviceCommandType;
import org.clangen.autom8.device.Lamp;

public class SetLampBrightness extends SendDeviceCommand {
    private String mDeviceAddress;
    private HashMap<String, String> mParams = new HashMap<>();

    public SetLampBrightness(Lamp lampDevice, int dimPercent) {
        super();
        mDeviceAddress = lampDevice.getAddress();
        mParams.put("brightness", String.valueOf(dimPercent));
    }

    @Override
    protected String getCommandName() {
        return "set_brightness";
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
