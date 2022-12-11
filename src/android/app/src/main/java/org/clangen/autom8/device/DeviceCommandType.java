package org.clangen.autom8.device;

public enum DeviceCommandType {
    PowerLine(0),
    RadioFrequency(1);

    private int mRawType;

    DeviceCommandType(int rawType) {
        mRawType = rawType;
    }

    public int toRawType() {
        return mRawType;
    }

    public String toString() {
        return String.valueOf(mRawType);
    }
}
