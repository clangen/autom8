package org.clangen.autom8.ui.activity;

import org.clangen.autom8.R;

public enum AdapterType {
    Flat(0, R.string.menu_list_flat),
    Grouped(1, R.string.menu_list_grouped);

    private int mId, mStringId;

    AdapterType(int id, int stringId) {
        mId = id;
        mStringId = stringId;
    }

    public static AdapterType fromId(int id) {
        switch (id) {
            case 0: return Flat;
            case 1: return Grouped;
        }

        throw new IllegalArgumentException("id is not valid");
    }

    public int getId() {
        return mId;
    }

    public int getStringId() {
        return mStringId;
    }
}
