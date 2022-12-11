package org.clangen.autom8.net.request;

import java.util.HashMap;

import org.clangen.autom8.device.DeviceCommandType;
import org.clangen.autom8.net.Message;
import org.clangen.autom8.net.MessageName;
import org.clangen.autom8.net.MessageType;
import org.json.JSONException;
import org.json.JSONObject;

import android.util.Log;

public abstract class SendDeviceCommand extends Message {
    @Override
    public String getBody() {
        if (mBody == null) {
            createBody();
        }

        return mBody;
    }

    @Override
    public String getName() {
        return MessageName.SendDeviceCommand.toRawName();
    }

    @Override
    public MessageType getType() {
        return MessageType.Request;
    }

    private void createBody() {
        try {
            JSONObject command = new JSONObject();
            command.put("name", getCommandName());
            command.put("type", getCommandType().toRawType());
            command.put("address", getDeviceAddress());

            JSONObject parameters = new JSONObject();
            HashMap<String, String> parameterMap = getCommandParams();
            if (parameterMap != null) {
                for (String key : parameterMap.keySet()) {
                    parameters.put(key, parameterMap.get(key));
                }
            }

            command.put("parameters", parameters);

            mJSONBody = new JSONObject();
            mJSONBody.put("command", command);

            mBody = mJSONBody.toString();
        }
        catch (JSONException ex) {
            Log.i("SendDeviceCommand", "unable to create JSON body!", ex);
        }
    }

    protected abstract DeviceCommandType getCommandType();
    protected abstract String getCommandName();
    protected abstract String getDeviceAddress();
    protected abstract HashMap<String, String> getCommandParams();
}
