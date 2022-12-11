package org.clangen.autom8.ui.dialog;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.View;
import android.widget.SeekBar;

import androidx.fragment.app.DialogFragment;

import org.clangen.autom8.R;

public class LampBrightnessDialog extends DialogFragment {
    public static final String TAG = "LampBrightnessDialog";
    public static final String DEVICE_ADDRESS = "org.clangen.autom8.DEVICE_ADDRESS";
    public static final String LAMP_BRIGHTNESS = "org.clangen.autom8.LAMP_BRIGHTNESS";

    private OnSetLampBrightnessListener mListener;

    public interface OnSetLampBrightnessListener {
        void onSetLampBrightness(String address, int brightness);
    }

    public LampBrightnessDialog() {
    }

    public void setOnSetLampBrightnessListener(OnSetLampBrightnessListener listener) {
        mListener = listener;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        final String address = getArguments().getString(DEVICE_ADDRESS);
        final int brightness = getArguments().getInt(LAMP_BRIGHTNESS);

        View dimView = View.inflate(getActivity(), R.layout.dim_lamp, null);

        final SeekBar seekBar = (SeekBar) dimView.findViewById(R.id.DimLampSeekBar);
        seekBar.setMax(100);
        seekBar.setProgress(brightness);

        DialogInterface.OnClickListener okClickListener = new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                if (mListener != null) {
                    mListener.onSetLampBrightness(address, seekBar.getProgress());
                }
            }
        };

        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        builder.setPositiveButton(R.string.button_ok, okClickListener);
        builder.setNegativeButton(R.string.button_cancel, null);
        builder.setTitle(getString(R.string.dlg_lamp_brightness_title));
        builder.setView(dimView);
        return builder.create();
    }
}
