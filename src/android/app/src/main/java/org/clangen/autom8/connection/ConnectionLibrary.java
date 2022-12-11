package org.clangen.autom8.connection;

import android.content.ContentValues;
import android.content.Context;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteDatabase.CursorFactory;
import android.database.sqlite.SQLiteOpenHelper;
import android.preference.PreferenceManager;

import org.clangen.autom8.R;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

public class ConnectionLibrary {
    private SQLiteDatabase mDatabase;

    private final static int CURRENT_DATABASE_VERSION = 2;
    private final static int DATABASE_VERSION_WITH_FINGERPRINT = 2;

    private static final String CONNECTION_TABLE = "connection";
    private static final String ID_COLUMN = "id";
    private static final String ALIAS_COLUMN = "alias";
    private static final String HOST_COLUMN = "host";
    private static final String PORT_COLUMN = "port";
    private static final String PASSWORD_COLUMN = "password";
    private static final String VERIFIED_COLUMN = "verified";
    private static final String FINGERPRINT_COLUMN = "fingerprint";

    private static ConnectionLibrary sInstance;

    private ConnectionLibrary(Context context) {
        loadDatabase(context);
    }

    public static synchronized ConnectionLibrary getInstance(Context context) {
        if (sInstance == null) {
            sInstance = new ConnectionLibrary(context);
        }

        return sInstance;
    }

    public static Connection getDefaultConnection(Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        Long defaultConnectionId = prefs.getLong(context.getString(R.string.pref_default_connection_id), 0);

        // try to load the default connection
        ConnectionLibrary cm = ConnectionLibrary.getInstance(context);
        Connection defaultConnection = cm.getConnectionById(defaultConnectionId);

        if (defaultConnection == null) {
            // default connection couldn't be loaded, try to load first connection
            List<Connection> connections = cm.getConnections();
            if (connections.size() > 0) {
                defaultConnection = connections.get(0);
            }
            else {
                return null;
            }
        }

        return defaultConnection;
    }

    public static void setDefaultConnection(Context context, long databaseId) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        SharedPreferences.Editor editor = prefs.edit();
        editor.putLong(context.getString(R.string.pref_default_connection_id), databaseId);
        editor.apply();
    }

    public boolean markConnectionUnverified(final long databaseId, final String fingerprint) {
        final ContentValues cv = new ContentValues();
        cv.put(VERIFIED_COLUMN, 0);
        cv.put(FINGERPRINT_COLUMN, fingerprint.toLowerCase());

        final String predicate = String.format(Locale.ENGLISH, "%s = ?", ID_COLUMN);

        int updated =
            mDatabase.update(
                CONNECTION_TABLE,
                cv,
                predicate,
                new String[] { String.valueOf(databaseId) });

        return updated > 0;
    }

    public boolean markConnectionVerified(final long databaseId) {
        final ContentValues cv = new ContentValues();
        cv.put(VERIFIED_COLUMN, 1);

        final String predicate = String.format(Locale.ENGLISH, "%s = ?", ID_COLUMN);

        int updated =
            mDatabase.update(
            CONNECTION_TABLE,
            cv,
            predicate,
            new String[] { String.valueOf(databaseId) });

        return updated > 0;
    }

    public List<Connection> getConnections() {
        ArrayList<Connection> result = new ArrayList<>();

        Cursor cursor = mDatabase.query(
            CONNECTION_TABLE,
            new String[] {
                ID_COLUMN,
                ALIAS_COLUMN,
                HOST_COLUMN,
                PORT_COLUMN,
                PASSWORD_COLUMN,
                VERIFIED_COLUMN,
                FINGERPRINT_COLUMN
            },
            null,             // where clause
            null,             // where clause's binded parameters
            null,             // group by
            null,             // having
            ALIAS_COLUMN);    // order

        if (cursor != null) {
            try {
                while (cursor.moveToNext()) {
                    result.add(new Connection(
                        cursor.getString(1),
                        cursor.getString(2),
                        cursor.getInt(3),
                        cursor.getString(4),
                        cursor.getLong(0),
                        cursor.getInt(5),
                        cursor.getString(6)));
                }
            }
            finally {
                cursor.close();
            }
        }

        return result;
    }

    public boolean exists(Connection connection) {
        return (getConnectionById(connection.getDatabaseId()) != null);
    }

    public boolean exists(Long databaseId) {
        return (getConnectionById(databaseId) != null);
    }

    public int count() {
        Cursor cursor = mDatabase.query(
            CONNECTION_TABLE,
            new String[] { "COUNT(*)" },
            null, null, null, null, null);

        try {
            if ((cursor != null) && (cursor.getCount() > 0)) {
                cursor.moveToNext();
                return cursor.getInt(0);
            }
        }
        finally {
            if (cursor != null) {
                cursor.close();
            }
        }

        return -1;
    }

    public Connection getConnectionById(Long databaseId) {
        if (databaseId != null) {
            Cursor cursor = mDatabase.query(
                CONNECTION_TABLE,
                new String[] {
                    ID_COLUMN,
                    ALIAS_COLUMN,
                    HOST_COLUMN,
                    PORT_COLUMN,
                    PASSWORD_COLUMN,
                    VERIFIED_COLUMN,
                    FINGERPRINT_COLUMN
                },
                String.format(Locale.ENGLISH, "%s=%d", ID_COLUMN, databaseId),
                null,   // bind
                null,   // group by
                null,   // having
                null);  // order

            try {
                if ((cursor != null) && (cursor.getCount() > 0)) {
                    cursor.moveToNext();

                    return new Connection(
                        cursor.getString(1),
                        cursor.getString(2),
                        cursor.getInt(3),
                        cursor.getString(4),
                        cursor.getLong(0),
                        cursor.getInt(5),
                        cursor.getString(6));
                }
            }
            finally {
                cursor.close();
            }
        }

        return null;
    }

    public long add(Connection connection) {
        if ( ! exists(connection)) {
            ContentValues values = contentValuesFromConnection(connection);
            return mDatabase.insert(CONNECTION_TABLE, ID_COLUMN, values);
        }

        return -1;
    }

    public boolean update(Long connectionToUpdateId, Connection connection) {
        if (exists(connectionToUpdateId)) {
            int rowsChanged = mDatabase.update(
                CONNECTION_TABLE,
                contentValuesFromConnection(connection),
                String.format(Locale.ENGLISH, "%s=%d", ID_COLUMN, connectionToUpdateId),
                null);

            return (rowsChanged > 0);
        }

        return false;
    }

    public boolean delete(Connection connection) {
        if (exists(connection)) {
            int rowsDeleted = mDatabase.delete(
                CONNECTION_TABLE,
                String.format(Locale.ENGLISH, "%s=%d", ID_COLUMN, connection.getDatabaseId()),
                null);

            return (rowsDeleted > 0);
        }

        return false;
    }

    private ContentValues contentValuesFromConnection(Connection connection) {
        ContentValues values = new ContentValues();
        values.put(ALIAS_COLUMN, connection.getAlias());
        values.put(HOST_COLUMN, connection.getHost());
        values.put(PORT_COLUMN, connection.getPort());
        values.put(PASSWORD_COLUMN, connection.getPassword());
        values.put(VERIFIED_COLUMN, connection.isVerified() ? 1 : 0);
        values.put(FINGERPRINT_COLUMN, connection.getFingerprint());
        return values;
    }

    private void loadDatabase(Context context) {
        DatabaseHelper dbHelper = new DatabaseHelper(context, "ConnectionDatabase", null, CURRENT_DATABASE_VERSION);
        mDatabase = dbHelper.getWritableDatabase();
    }

    private class DatabaseHelper extends SQLiteOpenHelper {
        public DatabaseHelper(Context context, String name, CursorFactory factory, int version) {
            super(context, name, factory, version);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            initializeTables(db);
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            if (oldVersion < DATABASE_VERSION_WITH_FINGERPRINT) {
                final String addFingerprintColumn = String.format(
                    Locale.ENGLISH,
                    "ALTER TABLE %s ADD COLUMN %s STRING;",
                    CONNECTION_TABLE, FINGERPRINT_COLUMN);

                final String addVerifiedColumn = String.format(
                    Locale.ENGLISH,
                    "ALTER TABLE %s ADD COLUMN %s INTEGER;",
                    CONNECTION_TABLE, VERIFIED_COLUMN);

                db.execSQL(addFingerprintColumn);
                db.execSQL(addVerifiedColumn);
            }
        }

        private void initializeTables(SQLiteDatabase db) {
            String sql =
                "CREATE TABLE IF NOT EXISTS %s (" +
                 "  %s INTEGER PRIMARY KEY AUTOINCREMENT," +    // id
                 "  %s STRING UNIQUE," +                        // alias
                 "  %s STRING," +                               // host
                 "  %s INTEGER," +                              // port
                 "  %s STRING," +                               // password
                 "  %S STRING," +                               // fingerprint
                 "  %S INTEGER);";                              // verified

            sql = String.format(sql, CONNECTION_TABLE, ID_COLUMN, ALIAS_COLUMN,
                HOST_COLUMN, PORT_COLUMN, PASSWORD_COLUMN, FINGERPRINT_COLUMN, VERIFIED_COLUMN);

            db.execSQL(sql);
        }
    }
}
