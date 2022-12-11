package org.clangen.autom8.ui.fragment;

import android.app.Activity;
import android.os.Bundle;
import android.os.RemoteException;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;

import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;

import org.clangen.autom8.R;
import org.clangen.autom8.device.Device;
import org.clangen.autom8.device.DeviceLibrary;
import org.clangen.autom8.device.DeviceLibraryFactory;
import org.clangen.autom8.device.DeviceStatus;
import org.clangen.autom8.device.DeviceType;
import org.clangen.autom8.device.DeviceUtil;
import org.clangen.autom8.device.Group;
import org.clangen.autom8.device.Lamp;
import org.clangen.autom8.device.SecuritySensor;
import org.clangen.autom8.net.Message;
import org.clangen.autom8.net.request.ArmSensor;
import org.clangen.autom8.net.request.ResetSensorStatus;
import org.clangen.autom8.net.request.SetDeviceStatus;
import org.clangen.autom8.net.request.SetLampBrightness;
import org.clangen.autom8.service.IClientService;
import org.clangen.autom8.ui.activity.AdapterType;
import org.clangen.autom8.ui.activity.ClientServiceProvider;
import org.clangen.autom8.ui.adapter.BaseDeviceModelAdapter;
import org.clangen.autom8.ui.adapter.DeviceGroupModelAdapter;
import org.clangen.autom8.ui.adapter.DeviceListModelAdapter;
import org.clangen.autom8.ui.dialog.ClearSensorAlertDialog;
import org.clangen.autom8.ui.dialog.DisarmSensorDialog;
import org.clangen.autom8.ui.dialog.LampBrightnessDialog;
import org.clangen.autom8.ui.model.BaseDeviceModel;
import org.clangen.autom8.util.ActivityUtil;

public class DeviceModelFragment extends Fragment {
    private final static String TAG = "DeviceModelFragment";
    public static final String ADAPTER_TYPE = "org.clangen.autom8.AdapterType";

    private ClientServiceProvider mClientServiceProvider;
    private BaseDeviceModelAdapter mListAdapter;
    private AbsListView mListView;
    private AdapterType mAdapterType = AdapterType.Flat;
    private DeviceLibrary mDeviceLibrary;

    public DeviceModelFragment() {
        super();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mDeviceLibrary = DeviceLibraryFactory.getInstance(getActivity());
        mClientServiceProvider = (ClientServiceProvider) getActivity();

        Bundle arguments = getArguments();
        if (arguments != null) {
            mAdapterType = AdapterType.fromId(
                getArguments().getInt(ADAPTER_TYPE,
                AdapterType.Flat.getId())
            );
        }

        restoreDialogEventHandlers();
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putInt(ADAPTER_TYPE, mAdapterType.getId());
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mListAdapter.close();
        mListView.setAdapter(null);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return init(inflater);
    }

    public AdapterType getAdapterType() {
        return mAdapterType;
    }

    private View init(LayoutInflater inflater) {
        final View view = inflater.inflate(R.layout.devices, null);

        mListView = (AbsListView) view.findViewById(R.id.DevicesListView);
        if (mListView == null) {
            mListView = (AbsListView) view.findViewById(R.id.DevicesGridView);
        }

        resetListAdapter();

        return view;
    }

    private void resetListAdapter() {
        if (mListAdapter != null) {
            mListAdapter.setOnDeviceClickHandler(null);
        }

        final Activity a = getActivity();
        if (ActivityUtil.isLandscape(a)) {
            mListAdapter = new DeviceListModelAdapter(a);
        }
        else if (mAdapterType == AdapterType.Flat) {
            mListAdapter = new DeviceListModelAdapter(a);
        }
        else if (mAdapterType == AdapterType.Grouped) {
            mListAdapter = new DeviceGroupModelAdapter(a);
        }
        else {
            throw new RuntimeException("unsupported adapter type requested??");
        }

        mListAdapter.setOnDeviceClickHandler(mOnDeviceClickHandler);
        mListView.setAdapter(mListAdapter);
        mListAdapter.notifyDataSetChanged();
    }

    protected boolean sendClientMessage(final Message message) {
        try {
            IClientService clientService  = mClientServiceProvider.getClientService();

            if (clientService != null) {
                clientService.sendMessage(message);
                return true;
            }
            else {
                Log.d(TAG, "sendClientMessage failed because mClientService is null");
            }
        }
        catch (RemoteException re) {
            Log.d(TAG, "sendClientMessage failed due to RemoteException", re);
        }

        return false;
    }

    private void showLightDimDialog(final Lamp lampDevice) {
        Bundle bundle = new Bundle();
        bundle.putString(LampBrightnessDialog.DEVICE_ADDRESS, lampDevice.getAddress());
        bundle.putInt(LampBrightnessDialog.LAMP_BRIGHTNESS, lampDevice.getBrightness());

        LampBrightnessDialog dialog = new LampBrightnessDialog();
        dialog.setArguments(bundle);
        dialog.setOnSetLampBrightnessListener(mSetLampBrightnessListener);
        dialog.show(getFragmentManager(), LampBrightnessDialog.TAG);
    }

    private void confirmClearAlert(final SecuritySensor sensor) {
        if (sensor.getStatus() != DeviceStatus.ON) {
            return;
        }

        Bundle bundle = new Bundle();
        bundle.putString(ClearSensorAlertDialog.SENSOR_ADDRESS, sensor.getAddress());

        ClearSensorAlertDialog dialog = new ClearSensorAlertDialog();
        dialog.setArguments(bundle);
        dialog.setOnClearSensorAlertListener(mClearSensorAlertDialogListener);
        dialog.show(getFragmentManager(), ClearSensorAlertDialog.TAG);
    }

    private void toggleArmSecuritySensor(final SecuritySensor sensor) {
        if (!sensor.isArmed()) {
            sendClientMessage(new ArmSensor(sensor, true));
            return;
        }

        Bundle bundle = new Bundle();
        bundle.putString(DisarmSensorDialog.SENSOR_ADDRESS, sensor.getAddress());

        DisarmSensorDialog dialog = new DisarmSensorDialog();
        dialog.setArguments(bundle);
        dialog.setOnDisarmSensorListener(mDisarmSensorListener);
        dialog.show(getFragmentManager(), DisarmSensorDialog.TAG);
    }

    private void onDeviceItemClicked(Device device) {
        final String address = device.getAddress();

        final BaseDeviceModel deviceModel = mListAdapter.getDeviceModel();
        if (!deviceModel.isUpdating(address)) {
            deviceModel.setUpdating(address);
            mListAdapter.notifyDataSetChanged();
        }

        int newStatus = (device.getStatus() == DeviceStatus.ON)
                ? DeviceStatus.OFF
                : DeviceStatus.ON;

        sendClientMessage(new SetDeviceStatus(device, newStatus));
    }

    private void onSecuritySensorItemClicked(SecuritySensor sensor) {
        if (sensor.isTripped()) {
            confirmClearAlert(sensor);
        }
        else {
            toggleArmSecuritySensor(sensor);
        }
    }

    private void restoreDialogEventHandlers() {
        final FragmentManager fm = getFragmentManager();

        Fragment fragment = fm.findFragmentByTag(ClearSensorAlertDialog.TAG);
        if (fragment != null) {
            ((ClearSensorAlertDialog) fragment).setOnClearSensorAlertListener(mClearSensorAlertDialogListener);
        }

        fragment = fm.findFragmentByTag(LampBrightnessDialog.TAG);
        if (fragment != null) {
            ((LampBrightnessDialog) fragment).setOnSetLampBrightnessListener(mSetLampBrightnessListener);
        }

        fragment = fm.findFragmentByTag(DisarmSensorDialog.TAG);
        if (fragment != null) {
            ((DisarmSensorDialog) fragment).setOnDisarmSensorListener(mDisarmSensorListener);
        }
    }

    private DisarmSensorDialog.OnDisarmSensorListener mDisarmSensorListener =
        new DisarmSensorDialog.OnDisarmSensorListener() {
            @Override
            public void onDisarmSensor(String address) {
                SecuritySensor sensor = (SecuritySensor) mDeviceLibrary.getDeviceByAddress(address);
                sendClientMessage(new ArmSensor(sensor, false));
            }
        };

    private LampBrightnessDialog.OnSetLampBrightnessListener mSetLampBrightnessListener =
        new LampBrightnessDialog.OnSetLampBrightnessListener() {
            @Override
            public void onSetLampBrightness(String address, int brightness) {
                Lamp lamp = (Lamp) mDeviceLibrary.getDeviceByAddress(address);
                sendClientMessage(new SetLampBrightness(lamp, brightness));
            }
        };

    private ClearSensorAlertDialog.OnClearSensorAlertListener mClearSensorAlertDialogListener =
        new ClearSensorAlertDialog.OnClearSensorAlertListener() {
            @Override
            public void onClearSensorAlert(String address) {
                SecuritySensor sensor = (SecuritySensor) mDeviceLibrary.getDeviceByAddress(address);
                sendClientMessage(new ResetSensorStatus(sensor));
            }
        };

    private BaseDeviceModelAdapter.OnDeviceClickHandler mOnDeviceClickHandler =
            new BaseDeviceModelAdapter.OnDeviceClickHandler() {
                @Override
                public void onDeviceClicked(Device device) {
                    switch (device.getType()) {
                        case DeviceType.SECURITY_SENSOR:
                            onSecuritySensorItemClicked((SecuritySensor) device);
                            break;

                        default:
                            onDeviceItemClicked(device);
                            break;
                    }
                }

                @Override
                public boolean onDeviceLongClicked(Device device) {
                    if ((device.getType() == DeviceType.LAMP)
                            && (device.getStatus() == DeviceStatus.ON)) {
                        showLightDimDialog((Lamp) device);
                        return true;
                    }

                    return false;
                }

                @Override
                public void onGroupToggleClicked(Group group, boolean checked) {
                    int status = checked ? DeviceStatus.ON : DeviceStatus.OFF;

                    for (Device device : group) {
                        if (DeviceUtil.isDeviceToggleable(device) && device.getStatus() != status) {
                            sendClientMessage(new SetDeviceStatus(device, status));
                        }
                    }
                }
            };
}
