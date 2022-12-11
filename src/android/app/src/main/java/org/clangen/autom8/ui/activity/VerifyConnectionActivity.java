package org.clangen.autom8.ui.activity;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.view.View;
import android.widget.TextView;

import androidx.annotation.Nullable;

import org.clangen.autom8.R;
import org.clangen.autom8.connection.Connection;
import org.clangen.autom8.connection.ConnectionLibrary;
import org.clangen.autom8.ui.view.ViewUtil;
import org.clangen.autom8.util.ToolbarUtil;

public class VerifyConnectionActivity extends ActivityBase {
    public static final int REQUEST_CODE = 1024;

    private Handler mHandler = new Handler();
    private ConnectionLibrary mConnectionLibrary;
    private Connection mConnection;

    public static void start(Activity parentActivity) {
        Intent intent = new Intent();
        intent.setClass(parentActivity, VerifyConnectionActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        intent.setAction(Intent.ACTION_VIEW);
        parentActivity.startActivityForResult(intent, REQUEST_CODE);
        parentActivity.overridePendingTransition(0, 0);
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        setTheme(R.style.GenericTranslucentTheme);

        super.onCreate(savedInstanceState);

        mConnectionLibrary = ConnectionLibrary.getInstance(this);
        mConnection = ConnectionLibrary.getDefaultConnection(this);
        setResult(RESULT_CANCELED);

        setContentView(R.layout.verify_connection);

        ToolbarUtil.initSolid(this);
    }

    @Override
    protected void onContentViewVisible() {
        final View root = findViewById(R.id.VerifyConnectionView);
        mHandler.post(() -> ViewUtil.performCircularAnimationIn(root));
    }

    @Override
    public void onResume() {
        super.onResume();

        if (mConnection == null) {
            cancelAndFinish();
        }
        else {
            initViews();
        }
    }

    private void cancelAndFinish() {
        setResult(RESULT_CANCELED);
        finish();
    }

    @Override
    public void onBackPressed() {
        final Intent intent = new Intent();
        setResult(RESULT_CANCELED, intent);
        finish();
    }

    private void initViews() {
        ((TextView) findViewById(R.id.VerifyName)).setText(mConnection.getAlias());
        ((TextView) findViewById(R.id.VerifyHost)).setText(mConnection.getHost());
        ((TextView) findViewById(R.id.VerifyFingerprint)).setText(mConnection.getFingerprint());

        setResult(RESULT_CANCELED);

        findViewById(R.id.VerifyButton).setOnClickListener((v) -> {
                mConnectionLibrary.markConnectionVerified(mConnection.getDatabaseId());
                setResult(RESULT_OK);
                finish();
            });

        findViewById(R.id.EditConnectionsButton).setOnClickListener((v) -> {
                ConnectionManagerActivity.start(VerifyConnectionActivity.this);
                setResult(RESULT_OK);
                finish();
            });
    }
}

