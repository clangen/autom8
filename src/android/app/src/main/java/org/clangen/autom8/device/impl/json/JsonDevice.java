package org.clangen.autom8.device.impl.json;

import android.util.Log;

import org.clangen.autom8.device.Device;
import org.clangen.autom8.device.DeviceLibrary;
import org.clangen.autom8.device.DeviceStatus;
import org.clangen.autom8.device.DeviceType;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.security.InvalidParameterException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;

public class JsonDevice implements Device {
    private static final String TAG = "JsonDevice";

    public static final String LABEL_NODE = "label";
    public static final String TYPE_NODE = "type";
    public static final String ADDRESS_NODE = "address";
    public static final String STATUS_NODE = "status";
    public static final String ATTRIBUTES_NODE = "attributes";
    public static final String GROUPS_NODE = "groups";

    private String mAddress, mLabel;
    private int mStatus = DeviceStatus.UNKNOWN, mType = DeviceType.UNKNOWN;
    private boolean mValid = false;
    private HashMap<String, String> mAttributes = new HashMap<>();
    private List<String> mGroups;
    private JSONObject mRaw;

    public JsonDevice(JSONObject json) {
        mRaw = json;
        copyFrom(json);
    }

    public boolean isValid() {
        return mValid;
    }

    public String getAddress() {
        return mAddress;
    }

    public String getLabel() {
        return mLabel;
    }

    public int getStatus() {
        return mStatus;
    }

    public int getType() {
        return mType;
    }

    @Override
    public List<String> getGroups() {
        synchronized (this) {
            return new ArrayList<>(mGroups);
        }
    }

    @SuppressWarnings("unchecked")
    protected HashMap<String, String> getAttributes() {
        return (HashMap<String, String>) mAttributes.clone();
    }

    private void parseGroups(JSONArray groups) throws JSONException {
        mGroups = new ArrayList<>();

        if (groups != null) {
            for (int i = 0; i < groups.length(); i++) {
                String groupName = groups.getString(i).trim();
                if (groupName.length() > 0) {
                    mGroups.add(groups.getString(i));
                }
                else {
                    Log.w(TAG, "parsed empty group name!");
                }
            }
        }

        mGroups = Collections.unmodifiableList(mGroups);
    }

    public synchronized final void copyFrom(JSONObject json) {
        init(json);
        reinitialize();
    }

    public synchronized final void copyFrom(Device d) {
        if (d instanceof JsonDevice) {
            copyFrom(((JsonDevice) d).getRaw());
        }
        else {
            throw new InvalidParameterException("expected a JsonDevice");
        }
    }

    protected void reinitialize() {
        /* for base class use */
    }

    protected final JSONObject getRaw() {
        return mRaw;
    }

    protected final void init(JSONObject json) {
        mValid = false;

        try {
            mAddress = json.getString(ADDRESS_NODE);
            mLabel = json.getString(LABEL_NODE);
            mStatus = json.getInt(STATUS_NODE);
            mType = json.getInt(TYPE_NODE);

            parseGroups(json.optJSONArray(GROUPS_NODE));

            JSONObject attributes = json.getJSONObject(ATTRIBUTES_NODE);
            JSONArray keys = attributes.names();

            if (keys != null) {
                String key;
                for (int i = 0; i < keys.length(); i++) {
                    key = keys.getString(i);
                    mAttributes.put(key, attributes.get(key).toString());
                }
            }

            mValid = true;
        }
        catch (JSONException ex) {
            Log.i("JSONDevice", "unable to parse device from JSON!", ex);
        }
    }

    public String getAttribute(String key) {
        return mAttributes.get(key);
    }

    public int getDisplayPriority() {
        return DeviceLibrary.DEFAULT_DISPLAY_PRIORITY;
    }
}
