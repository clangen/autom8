package org.clangen.autom8.device;

import org.clangen.autom8.device.impl.json.JsonDevice;
import org.clangen.autom8.device.impl.json.JsonLamp;
import org.clangen.autom8.device.impl.json.JsonSecuritySensor;
import org.json.JSONException;
import org.json.JSONObject;

public class DeviceFactory {
    public static Device fromJson(JSONObject json) throws JSONException {
        int type = json.getInt(JsonDevice.TYPE_NODE);

        switch (type) {
            case DeviceType.APPLIANCE:
            case DeviceType.UNKNOWN:
                return new JsonDevice(json);

            case DeviceType.LAMP:
                return new JsonLamp(json);

            case DeviceType.SECURITY_SENSOR:
                return new JsonSecuritySensor(json);
        }

        return null;
    }
}
