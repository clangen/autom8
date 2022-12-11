package org.clangen.autom8.service;

import android.content.Context;
import android.os.PowerManager;
import android.util.Log;

public class ServiceWakeLock {
    private static final String TAG = "ServiceWakeLock";

    private PowerManager.WakeLock mWakeLock;
    private String mName = "(unnamed)";

    public ServiceWakeLock(String name) {
        mName = name;
    }

    public void acquire(Context context) {
        synchronized(this) {
            if(mWakeLock == null) {
                PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);

                mWakeLock = pm.newWakeLock(
                    PowerManager.PARTIAL_WAKE_LOCK |
                    PowerManager.ACQUIRE_CAUSES_WAKEUP,
                    mName);

                mWakeLock.acquire();

                Log.i(TAG, "\"" + mName + "\" WakeLock acquired");
            }
        }
    }

    public void release() {
        synchronized (this) {
            if (mWakeLock != null) {
                mWakeLock.release();
                mWakeLock = null;

                Log.i(TAG, "\"" + mName + "\" WakeLock released");
            }
        }
    }
}
