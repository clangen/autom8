package org.clangen.autom8.connection;

public class Connection {
    private String mAlias, mHost, mPassword, mFingerprint;
    private int mPort = 0, mVerified = 0;
    private Long mDatabaseId = null;
    private volatile int mHashCode = 0;

    public Connection(String alias, String host, int port, String password, int verified, String fingerprint) {
        mAlias = alias;
        mHost = host;
        mPort = port;
        mPassword = password;
        mVerified = verified;
        mFingerprint = fingerprint;
    }

    public Connection(String alias, String host, int port, String password, Long databaseId, int verified, String fingerprint) {
        this(alias, host, port, password, verified, fingerprint);
        mDatabaseId = databaseId;
    }

    public String getAlias() {
        return mAlias;
    }

    public String getHost() {
        return mHost;
    }

    public int getPort() {
        return mPort;
    }

    public String getPassword() {
        return mPassword;
    }

    public Long getDatabaseId() {
        return mDatabaseId;
    }

    public boolean isVerified() { return mVerified != 0; }

    public String getFingerprint() { return mFingerprint == null ? "" : mFingerprint; }

    @Override
    public boolean equals(Object o) {
        if (!(o instanceof Connection)) {
            return false;
        }

        Connection connection = (Connection) o;

        return (
           (connection.getDatabaseId() == mDatabaseId)
        && (connection.getAlias().equals(mAlias))
        && (connection.getHost().equals(mHost))
        && (connection.getPort() == mPort)
        && (connection.getPassword().equals(mPassword)))
        && (connection.getFingerprint().equals(mFingerprint))
        && (connection.isVerified() == isVerified());
    }

    @Override
    public int hashCode() {
        int result = mHashCode;

        if (result == 0) {
            result = 17;

            if (mDatabaseId != 0) {
                result = 31 * result + (int) (mDatabaseId ^ (mDatabaseId >>> 32));
            }

            result = 31 * result + mPort;
            result = 31 * result + mVerified;
            result = 31 * result + mAlias.hashCode();
            result = 31 * result + mHost.hashCode();
            result = 31 * result + mPassword.hashCode();
            result = 31 * result + mFingerprint.hashCode();

            mHashCode = result;
        }

        return result;
    }
}
