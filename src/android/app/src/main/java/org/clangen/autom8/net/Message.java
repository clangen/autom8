package org.clangen.autom8.net;

import java.io.UnsupportedEncodingException;

import org.clangen.autom8.Base64;
import org.json.JSONException;
import org.json.JSONObject;

import android.os.Parcel;
import android.os.Parcelable;

public class Message implements Parcelable {
    private String mName;
    private MessageType mType;
    protected String mBody;
    protected JSONObject mJSONBody;

    private static final String MESSAGE_URI_PREFIX = "autom8://";
    private static final String MESSAGE_URI_DELIMITER = "\r\n";
    private static final String ENCODING = "UTF-8";
    private static final String END_OF_MESSAGE = "\0";

    public static Message create(String rawBase64Text) {
        try {
            String plainText = new String(Base64.decode(rawBase64Text), ENCODING);

            String[] messageParts = plainText.split(MESSAGE_URI_DELIMITER);
            if (messageParts.length > 0) {
                String uri = messageParts[0];
                String body = (messageParts.length > 1) ? messageParts[1] : "";

                // validate URI
                if (uri.startsWith(MESSAGE_URI_PREFIX)) {
                    String[] uriParts = uri.substring(
                        MESSAGE_URI_PREFIX.length()).split("/");;

                    if (uriParts.length == 2) {
                        return new Message(
                            MessageType.fromRawType(uriParts[0]),
                            uriParts[1],
                            body);
                    }
                }
            }
        }
        catch (UnsupportedEncodingException ex) {
            System.out.println("Client.receiveMessage: malformed UTF-8 message");
        }

        return null;
    }

    public Message() {
    }

    public Message(MessageType type, String name, String body) {
        mType = type;
        mName = name;
        mBody = body;
    }

    protected Message(Parcel parcel) {
        readFromParcel(parcel);
    }

    public MessageType getType() {
        return mType;
    }

    public String getName() {
        return mName;
    }

    public String getBody() {
        return mBody;
    }

    public JSONObject bodyToJSON() {
        if (mJSONBody == null) {
            try {
                mJSONBody = new JSONObject(mBody);
            }
            catch (JSONException ex) {

            }
        }

        return mJSONBody;
    }

    public String toString() {
        String messageString = String.format(
            "autom8://%s/%s\r\n%s",
            getType(),
            getName(),
            getBody());

        try {
            messageString = Base64.encodeBytes(
                messageString.getBytes(ENCODING),
                Base64.DONT_BREAK_LINES);

            messageString += Message.END_OF_MESSAGE;
        }
        catch (UnsupportedEncodingException ex) {
            throw new RuntimeException("Could not encode message to Base64!");
        }

        return messageString;
    }

    public void readFromParcel(Parcel source) {
        mType = MessageType.fromRawType(source.readString());
        mName = source.readString();
        mBody = source.readString();
    }

    public void writeToParcel(Parcel out, int describeContents) {
        out.writeString(mType.toRawType());
        out.writeString(mName);
        out.writeString(mBody);
    }

    public int describeContents() {
        return 0;
    }

    public static final Parcelable.Creator<Message> CREATOR = new Parcelable.Creator<Message>() {
        public Message createFromParcel(Parcel source) {
            return new Message(source);
        }

        public Message[] newArray(int size) {
            return new Message[size];
        }
    };
}
