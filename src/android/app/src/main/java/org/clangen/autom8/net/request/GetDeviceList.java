package org.clangen.autom8.net.request;

import org.clangen.autom8.net.Message;
import org.clangen.autom8.net.MessageType;

public class GetDeviceList extends Message {
    public GetDeviceList() {
        super();
    }

    @Override
    public String getBody() {
        return "{ }";
    }

    @Override
    public String getName() {
        return "get_device_list";
    }

    @Override
    public MessageType getType() {
        return MessageType.Request;
    }
}
