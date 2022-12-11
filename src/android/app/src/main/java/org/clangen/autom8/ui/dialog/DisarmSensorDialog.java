package org.clangen.autom8.ui.dialog;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;

import androidx.fragment.app.DialogFragment;

import org.clangen.autom8.R;

public class DisarmSensorDialog extends DialogFragment {
    public static final String TAG = "DisarmSensorDialog";
    public static final String SENSOR_ADDRESS = "org.clangen.autom8.SENSOR_ADDRESS";

    private OnDisarmSensorListener mListener;

    public interface OnDisarmSensorListener {
        void onDisarmSensor(String address);
    }

    public DisarmSensorDialog() {
    }

    public void setOnDisarmSensorListener(OnDisarmSensorListener listener) {
        mListener = listener;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        final String address = getArguments().getString(SENSOR_ADDRESS);

        DialogInterface.OnClickListener yesClickListener = new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                if (mListener != null) {
                    mListener.onDisarmSensor(address);
                }
            }
        };

        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        builder.setPositiveButton(R.string.button_yes, yesClickListener);
        builder.setNegativeButton(R.string.button_no, null);
        builder.setTitle(R.string.dlg_disarm_title);
        builder.setMessage(R.string.dlg_disarm_desc);
        return builder.create();
    }
}
