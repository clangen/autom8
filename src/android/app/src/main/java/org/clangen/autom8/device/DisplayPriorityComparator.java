package org.clangen.autom8.device;

import java.util.Comparator;

public class DisplayPriorityComparator implements Comparator<Device> {
    @Override
    public int compare(Device d1, Device d2) {
        /* sort by priority first (descending) */
        int d1dp = DeviceLibrary.MAXIMUM_DISPLAY_PRIORITY - d1.getDisplayPriority();
        int d2dp = DeviceLibrary.MAXIMUM_DISPLAY_PRIORITY - d2.getDisplayPriority();

        /* if priority matches, sort by label (ascending) */
        String key1 = String.format("%04d-%s", d1dp, d1.getLabel());
        String key2 = String.format("%04d-%s", d2dp, d2.getLabel());

        return key1.compareTo(key2);
    }
}
