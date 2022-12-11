package org.clangen.autom8.ui.adapter;

import android.app.Service;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.CompoundButton;
import android.widget.TextView;
import android.widget.ToggleButton;

import org.clangen.autom8.R;
import org.clangen.autom8.device.Device;
import org.clangen.autom8.device.DeviceStatus;
import org.clangen.autom8.device.DeviceType;
import org.clangen.autom8.device.Group;
import org.clangen.autom8.device.SecuritySensor;
import org.clangen.autom8.ui.model.BaseDeviceModel;
import org.clangen.autom8.ui.model.DeviceListModel;

public abstract class BaseDeviceModelAdapter extends BaseAdapter {
    private Context mContext;
    private BaseDeviceModel mDeviceModel;
    private LayoutInflater mLayoutInflater;
    private OnDeviceClickHandler mClickHandler;

    protected static class ItemViewHolder {
        public Device mDevice;
        public View mMainContent;
        public View mSeparator;
        public TextView mSeparatorText;
        public TextView mModuleInfoView;
        public TextView mLabelView;
        public View mUpdatingView;
        public TextView mStatusText;
        public ToggleButton mToggleButton;
    }

    public interface OnDeviceClickHandler {
        void onDeviceClicked(Device device);
        boolean onDeviceLongClicked(Device device);
        void onGroupToggleClicked(Group group, boolean checked);
    }

    public BaseDeviceModelAdapter(Context context) {
        mContext = context;
        mLayoutInflater = (LayoutInflater) mContext.getSystemService(Service.LAYOUT_INFLATER_SERVICE);
    }

    public void setOnDeviceClickHandler(OnDeviceClickHandler handler) {
        mClickHandler = handler;
    }

    protected void setModel(BaseDeviceModel deviceModel) {
        mDeviceModel = deviceModel;
        mDeviceModel.requery();
        mDeviceModel.setOnChangedListener(mOnDeviceModelChanged);
    }

    public BaseDeviceModel getDeviceModel() {
        return mDeviceModel;
    }

    public void close() {
        mDeviceModel.close();
    }

    public int getCount() {
        return (mDeviceModel == null ? 0 : mDeviceModel.size());
    }

    public Object getItem(int position) {
        return (mDeviceModel == null ? null : mDeviceModel.get(position));
    }

    public long getItemId(int position) {
        return position;
    }

    public View getView(int position, View convertView, ViewGroup parent) {
        if ((convertView == null) || (convertView.getId() != R.id.DeviceItemView)) {
            convertView = mLayoutInflater.inflate(R.layout.device_item, null, false);

            ItemViewHolder holder = new ItemViewHolder();
            holder.mMainContent = convertView.findViewById(R.id.DeviceItemMainContent);
            holder.mSeparator = convertView.findViewById(R.id.DeviceItemSeparator);
            holder.mSeparatorText = (TextView) convertView.findViewById(R.id.DeviceItemSeparatorText);
            holder.mStatusText = (TextView) convertView.findViewById(R.id.DeviceItemStatusViewText);
            holder.mLabelView = (TextView) convertView.findViewById(R.id.DeviceItemLabelView);
            holder.mModuleInfoView = (TextView) convertView.findViewById(R.id.DeviceItemModuleInfoView);
            holder.mUpdatingView = convertView.findViewById(R.id.DeviceItemUpdatingView);
            holder.mToggleButton = (ToggleButton) convertView.findViewById(R.id.DeviceItemGroupToggleView);

            convertView.setTag(holder);
        }

        final ItemViewHolder holder = (ItemViewHolder) convertView.getTag();
        final Device device = mDeviceModel.get(position);
        holder.mDevice = device;

        convertView.setOnClickListener(mOnDeviceClickListener);
        convertView.setOnLongClickListener(mOnDeviceLongClickListener);
        convertView.setTag(holder);

        holder.mSeparator.setVisibility(View.GONE);

        /* IMPORTANT!: set the toggle button's listener to nothing until the
        bind is complete, otherwise setting the state for this particular
        view will trigger the callback and lead to a crazy cascade of toggling
        due to constant rebinds */
        holder.mToggleButton.setOnCheckedChangeListener(null);
        holder.mToggleButton.setTag(null);

        if (mDeviceModel != null) {
            bindDeviceToView(convertView, device, position);
        }

        holder.mToggleButton.setOnCheckedChangeListener(mOnGroupToggleListener);
        holder.mToggleButton.setOnLongClickListener(mOnToggleButtonLongClickListener);

        return convertView;
    }

    protected void bindDeviceToView(View deviceView, Device device, int position) {
        ItemViewHolder holder = (ItemViewHolder) deviceView.getTag();

        holder.mLabelView.setText(device.getLabel());
        holder.mStatusText.setTextSize(18.0f);

        // String.format() is very inefficient
        holder.mModuleInfoView.setText(
            deviceTypeToString(device.getType()) + " " + device.getAddress());

        if (mDeviceModel.isUpdating(device.getAddress())) {
            holder.mUpdatingView.setVisibility(View.VISIBLE);
            holder.mStatusText.setVisibility(View.GONE);

            if (device.getStatus() == DeviceStatus.ON) {
                holder.mMainContent.setBackgroundColor(Color.TRANSPARENT);
            }
            else {
                holder.mMainContent.setBackgroundResource(R.drawable.device_item_on_background);
            }
        }
        else {
            holder.mUpdatingView.setVisibility(View.GONE);
            holder.mStatusText.setVisibility(View.VISIBLE);

            switch (device.getType()) {
                case DeviceType.SECURITY_SENSOR:
                    applySensorStyle((SecuritySensor) device, deviceView);
                    break;

                default:
                    applyDefaultStyle(device, deviceView);
                    break;
            }
        }
    }

    protected void applyDefaultStyle(Device device, View itemView) {
        final ItemViewHolder holder = (ItemViewHolder) itemView.getTag();
        final Resources r = mContext.getResources();

        int status = R.string.device_status_off;

        switch (device.getStatus()) {
            case DeviceStatus.ON:
                status = R.string.device_status_on;
                holder.mStatusText.setTextColor(r.getColor(R.color.device_row_on_status_text));
                holder.mStatusText.setBackgroundColor(r.getColor(R.color.device_row_on_status_bg));
                holder.mMainContent.setBackgroundResource(R.drawable.device_item_on_background);
                break;

            case DeviceStatus.OFF:
            case DeviceStatus.UNKNOWN:
                holder.mStatusText.setTextColor(r.getColor(R.color.device_row_off_status_text));
                holder.mStatusText.setBackgroundColor(r.getColor(R.color.device_row_off_status_bg));
                holder.mMainContent.setBackgroundColor(Color.TRANSPARENT);
                break;
        }

        holder.mStatusText.setText(status);
    }

    protected void applySensorStyle(SecuritySensor sensor, View itemView) {
        final ItemViewHolder holder = (ItemViewHolder) itemView.getTag();
        final Resources r = mContext.getResources();

        int status = R.string.device_status_off;

        if (sensor.isTripped()) {
            status = R.string.device_status_alert;
            holder.mStatusText.setTextColor(r.getColor(R.color.device_row_alert_status_text));
            holder.mStatusText.setBackgroundColor(r.getColor(R.color.device_row_alert_status_bg));
            holder.mMainContent.setBackgroundResource(R.drawable.device_item_alert_background);
        }
        else if (sensor.isArmed()) {
            status = R.string.device_status_armed;
            holder.mStatusText.setTextSize(16.0f);
            holder.mStatusText.setTextColor(r.getColor(R.color.device_row_on_status_text));
            holder.mStatusText.setBackgroundColor(r.getColor(R.color.device_row_on_status_bg));
            holder.mMainContent.setBackgroundResource(R.drawable.device_item_on_background);
        }
        else {
            holder.mStatusText.setTextColor(r.getColor(R.color.device_row_off_status_text));
            holder.mStatusText.setBackgroundColor(r.getColor(R.color.device_row_off_status_bg));
            holder.mMainContent.setBackgroundColor(Color.TRANSPARENT);
        }

        holder.mStatusText.setText(status);
    }

    protected String deviceTypeToString(int deviceType) {
        switch (deviceType) {
            case DeviceType.LAMP: return mContext.getString(R.string.device_type_lamp);
            case DeviceType.APPLIANCE: return mContext.getString(R.string.device_type_appliance);
            case DeviceType.SECURITY_SENSOR: return mContext.getString(R.string.device_type_sensor);
            default: return mContext.getString(R.string.device_type_unknown);
        }
    }

    protected void onDeviceModelChanged() {
        notifyDataSetChanged();
    }

    private View.OnLongClickListener mOnToggleButtonLongClickListener =
        new View.OnLongClickListener() {
            @Override
            public boolean onLongClick(View view) {
                if (mClickHandler != null && view.getTag() != null) {
                    mClickHandler.onGroupToggleClicked((Group) view.getTag(), true);
                    return true;
                }

                return false;
            }
        };

    private View.OnLongClickListener mOnDeviceLongClickListener =
        new View.OnLongClickListener() {
            @Override
            public boolean onLongClick(View view) {
                if (mClickHandler != null && view.getTag() != null) {
                    final ItemViewHolder vh = (ItemViewHolder) view.getTag();
                    return mClickHandler.onDeviceLongClicked(vh.mDevice);
                }

                return false;
            }
        };

    private View.OnClickListener mOnDeviceClickListener =
        new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (mClickHandler != null && view.getTag() != null) {
                    final ItemViewHolder vh = (ItemViewHolder) view.getTag();
                    mClickHandler.onDeviceClicked(vh.mDevice);
                }
            }
        };

    private CompoundButton.OnCheckedChangeListener mOnGroupToggleListener =
        new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean checked) {
                if (mClickHandler != null && compoundButton.getTag() != null) {
                    mClickHandler.onGroupToggleClicked((Group) compoundButton.getTag(), checked);
                }
            }
        };

    private DeviceListModel.OnChangedListener mOnDeviceModelChanged =
        new DeviceListModel.OnChangedListener() {
            public void onChanged() {
                onDeviceModelChanged();
            }
        };
}
