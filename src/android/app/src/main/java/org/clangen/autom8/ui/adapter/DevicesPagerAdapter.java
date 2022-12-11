package org.clangen.autom8.ui.adapter;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.view.ViewGroup;

import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentPagerAdapter;

import org.clangen.autom8.device.DeviceLibrary;
import org.clangen.autom8.device.DeviceLibraryFactory;
import org.clangen.autom8.ui.activity.AdapterType;
import org.clangen.autom8.ui.fragment.DeviceModelFragment;
import org.clangen.autom8.util.ActivityUtil;

import java.util.HashMap;

public class DevicesPagerAdapter extends FragmentPagerAdapter {
    private static final AdapterType[] PAGE_ORDER_FLAT_AND_GROUPED =
        new AdapterType[] { AdapterType.Flat, AdapterType.Grouped };

    private static final AdapterType[] PAGE_ORDER_FLAT_ONLY =
        new AdapterType[] { AdapterType.Flat };

    private static final AdapterType[] EMPTY = new AdapterType[] { };

    private static final IntentFilter INTENT_FILTER = new IntentFilter();

    private Context mContext;
    private HashMap<Integer, DeviceModelFragment> mItems;
    private DeviceLibrary mDeviceLibrary;
    private AdapterType[] mPages = EMPTY;
    private OnResetListener mOnResetListener;
    private Handler mHandler = new Handler();

    public interface OnResetListener {
        void onReset();
    }

    static {
        INTENT_FILTER.addAction(DeviceLibrary.ACTION_DEVICE_LIBRARY_REFRESHED);
    }

    public DevicesPagerAdapter(Activity context, FragmentManager fm) {
        super(fm);
        this.mContext = context;
        mItems = new HashMap<>();
        mDeviceLibrary = DeviceLibraryFactory.getInstance(context.getApplicationContext());
        ActivityUtil.registerReceiver(context, mLibraryRefreshedReceiver, INTENT_FILTER);

        /* wait until the next pass through the event loop so the caller
        has a chance to register a reset handler */
        mHandler.post(new Runnable() {
            public void run() {
                updatePageOrder();
            }
        });
    }

    public void onDestroy() {
        ActivityUtil.unregisterReceiver(mContext, mLibraryRefreshedReceiver);
        mOnResetListener = null;
    }

    public void setOnInitializedListener(OnResetListener listener) {
        mOnResetListener = listener;
    }

    @Override
    public int getCount() {
        return mPages.length;
    }

    @Override
    public Fragment getItem(int position) {
        DeviceModelFragment fragment = mItems.get(position);

        if (fragment != null) {
            return fragment;
        }

        Bundle arguments = new Bundle();
        arguments.putInt(DeviceModelFragment.ADAPTER_TYPE, mPages[position].getId());

        fragment = new DeviceModelFragment();
        fragment.setArguments(arguments);

        mItems.put(position, fragment);

        return fragment;
    }

    public int getIndexForType(AdapterType type) {
        for (int i = 0; i < mPages.length; i++) {
            if (mPages[i] == type) {
                return i;
            }
        }
        return -1;
    }

    public AdapterType getAdapterType(int position) {
        final DeviceModelFragment f = mItems.get(position);
        return f == null ? AdapterType.Flat : f.getAdapterType();
    }

    @Override
    public CharSequence getPageTitle(int position) {
        return mContext.getString(mPages[position].getStringId());
    }

    @Override
    public Object instantiateItem(ViewGroup container, int position) {
        DeviceModelFragment fragment =
            (DeviceModelFragment) super.instantiateItem(container, position);

        mItems.put(position, fragment);

        return fragment;
    }

    private void updatePageOrder() {
        AdapterType[] pages = PAGE_ORDER_FLAT_AND_GROUPED;
        if (ActivityUtil.isLandscape(mContext) || mDeviceLibrary.getDeviceGroups().size() == 0) {
            pages = PAGE_ORDER_FLAT_ONLY;
        }

        if (pages != mPages) {
            mPages = pages;
            this.notifyDataSetChanged();

            if (mOnResetListener != null) {
                mOnResetListener.onReset();
            }
        }
    }

    private BroadcastReceiver mLibraryRefreshedReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            updatePageOrder();
        }
    };
}
