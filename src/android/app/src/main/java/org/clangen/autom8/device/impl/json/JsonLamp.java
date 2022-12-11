package org.clangen.autom8.device.impl.json;

import org.clangen.autom8.device.Lamp;
import org.json.JSONObject;

public class JsonLamp extends JsonDevice implements Lamp {
    private static final String BRIGHTNESS_ATTRIBUTE = "brightness";

    private int mBrightness;

    public JsonLamp(JSONObject json) {
        super(json);
    }

    @Override
    public int getBrightness() {
        return mBrightness;
    }

    @Override
    protected void reinitialize() {
        super.reinitialize();
        mBrightness = Integer.parseInt(getAttribute(BRIGHTNESS_ATTRIBUTE));
    }
}
