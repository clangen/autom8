package org.clangen.autom8.device.impl.json;

import org.clangen.autom8.device.DeviceLibrary;
import org.clangen.autom8.device.SecuritySensor;
import org.json.JSONObject;

public class JsonSecuritySensor extends JsonDevice implements SecuritySensor {
    private static final String ARMED_ATTRIBUTE = "armed";
    private static final String TRIPPED_ATTRIBUTE = "tripped";

    private boolean mTripped, mArmed;

    public JsonSecuritySensor(JSONObject json) {
        super(json);
    }

    @Override
    public boolean isTripped() {
        return mTripped;
    }

    @Override
    public boolean isArmed() {
        return mArmed;
    }

    @Override
    public int getDisplayPriority() {
        if (mTripped && mArmed) {
            return DeviceLibrary.SECURITY_ALERT_DISPLAY_PRIORITY;
        }

        return DeviceLibrary.SECURITY_SENSOR_DISPLAY_PRIORITY;
    }

    @Override
    protected void reinitialize() {
        super.reinitialize();
        mArmed = "true".equals(getAttribute(ARMED_ATTRIBUTE));
        mTripped = "true".equals(getAttribute(TRIPPED_ATTRIBUTE));
    }
}
