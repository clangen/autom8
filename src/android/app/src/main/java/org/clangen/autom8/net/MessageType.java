package org.clangen.autom8.net;

public enum MessageType {
    Request("request"),
    Response("response");

    private String mRawType;

    MessageType(String rawType) {
        mRawType = rawType;
    }

    public String toRawType() {
        return mRawType;
    }

    public String toString() {
        return mRawType;
    }

    boolean is(String rawType) {
        return mRawType.equals(rawType);
    }

    public static MessageType fromRawType(String rawType) {
        if (Request.is(rawType)) return Request;
        if (Response.is(rawType)) return Response;
        throw new IllegalArgumentException("Unknown message type");
    }
}
