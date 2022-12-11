package org.clangen.autom8.device;

import java.util.List;

public interface Device {
    int getType();
    int getStatus();
    String getAddress();
    String getLabel();
    String getAttribute(String key);
    int getDisplayPriority();
    List<String> getGroups();
    void copyFrom(Device d);
}
