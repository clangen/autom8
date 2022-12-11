package org.clangen.autom8.ui.activity;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;

import org.clangen.autom8.R;
import org.clangen.autom8.util.ToolbarUtil;

public class SettingsActivity extends AppCompatActivity {
    public static void start(Activity parentActivity) {
        Intent intent = new Intent();
        intent.setClass(parentActivity, SettingsActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        intent.setAction(Intent.ACTION_VIEW);
        parentActivity.startActivityForResult(intent, 0);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.settings_activity);

        ToolbarUtil.initSolid(this);
    }
}
