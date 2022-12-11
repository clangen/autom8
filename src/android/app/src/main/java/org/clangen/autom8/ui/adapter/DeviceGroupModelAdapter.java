package org.clangen.autom8.ui.adapter;

import android.content.Context;
import android.view.View;

import org.clangen.autom8.device.Device;
import org.clangen.autom8.device.Group;
import org.clangen.autom8.ui.model.DeviceGroupModel;

public class DeviceGroupModelAdapter extends BaseDeviceModelAdapter {
    private DeviceGroupModel mGroupModel;

    public DeviceGroupModelAdapter(Context context) {
        super(context);

        mGroupModel = new DeviceGroupModel(context);
        setModel(mGroupModel);
    }

    @Override
    protected void bindDeviceToView(View deviceView, Device device, int position) {
        super.bindDeviceToView(deviceView, device, position);

        ItemViewHolder holder = (ItemViewHolder) deviceView.getTag();

        Group group = mGroupModel.getDelimiter(position);
        if (group != null) {
            holder.mSeparatorText.setText(group.getName());
            holder.mSeparator.setVisibility(View.VISIBLE);
            holder.mToggleButton.setEnabled(group.getToggleableDeviceCount() > 0);
            holder.mToggleButton.setTag(group);
            holder.mToggleButton.setChecked(group.atLeastOneToggleableDeviceOn());
        }
        else {
            holder.mSeparator.setVisibility(View.GONE);
        }
    }
}
