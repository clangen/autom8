package org.clangen.autom8.ui.dialog;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;

import androidx.fragment.app.DialogFragment;

import org.clangen.autom8.R;

public class ClearSensorAlertDialog extends DialogFragment {
    public static final String TAG = "ClearSensorAlertDialog";
    public static final String SENSOR_ADDRESS = "org.clangen.autom8.SENSOR_ADDRESS";

    private OnClearSensorAlertListener mListener;

    public interface OnClearSensorAlertListener {
        void onClearSensorAlert(String address);
    }

    public ClearSensorAlertDialog() {
    }

    public void setOnClearSensorAlertListener(OnClearSensorAlertListener listener) {
        mListener = listener;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        final String address = getArguments().getString(SENSOR_ADDRESS);

        DialogInterface.OnClickListener yesClickListener = new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                if (mListener != null) {
                    mListener.onClearSensorAlert(address);
                }
            }
        };

        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        builder.setPositiveButton(R.string.button_yes, yesClickListener);
        builder.setNegativeButton(R.string.button_no, null);
        builder.setTitle(R.string.dlg_reset_alert_title);
        builder.setMessage(R.string.dlg_reset_alert_desc);
        return builder.create();
    }
}
