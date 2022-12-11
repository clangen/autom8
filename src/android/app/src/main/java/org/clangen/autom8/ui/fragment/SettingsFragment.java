package org.clangen.autom8.ui.fragment;

import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.util.Log;

import org.clangen.autom8.R;
import org.clangen.autom8.service.ClientService;
import org.clangen.autom8.ui.activity.ConnectionManagerActivity;

public class SettingsFragment extends PreferenceFragment {
    public static final String ACTION_TRANSLUCENCY_TOGGLED = "org.clangen.autom8.ACTION_TRANSLUCENCY_TOGGLED";

    @Override
    public boolean onPreferenceTreeClick(
            PreferenceScreen preferenceScreen, Preference p)
    {
        boolean result = super.onPreferenceTreeClick(preferenceScreen, p);

        if (p == getTranslucencyPreference()) {
            showTranslucencyToggledDialog();
        }
        else if (p == getPeristencePreference()) {
            final Intent intent = new Intent(getActivity(), ClientService.class);
            intent.setAction(ClientService.ACTION_START_SERVICE);
            getActivity().startService(intent);
        }
        else if (p == getConnectionManagerPreference()) {
            ConnectionManagerActivity.start(getActivity());
            return false;
        }

        getActivity().sendBroadcast(new Intent(ClientService.ACTION_RELOAD_SETTINGS));

        return result;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.settings);
        checkEnableTranslucency();
        setVersionPreference();
    }

    @Override
    public void onDestroy() {
        getActivity().sendBroadcast(new Intent(ClientService.ACTION_RELOAD_SETTINGS));
        super.onDestroy();
    }

    private void checkEnableTranslucency() {
        getTranslucencyPreference()
                .setEnabled(android.os.Build.VERSION.SDK_INT >= 5);
    }

    private Preference getTranslucencyPreference() {
        return findPreference(getString(R.string.pref_translucency_enabled));
    }

    private Preference getPeristencePreference() {
        return findPreference(getString(R.string.pref_run_in_background));
    }

    private Preference getConnectionManagerPreference() {
        return findPreference(getString(R.string.pref_connection_manager));
    }

    private void showTranslucencyToggledDialog() {
        getActivity().sendBroadcast(new Intent(ACTION_TRANSLUCENCY_TOGGLED));
    }

    private void setVersionPreference() {
        PackageManager pm = getActivity().getPackageManager();

        try {
            PackageInfo info = pm.getPackageInfo(getActivity().getPackageName(), 0);

            Preference p = findPreference(getString(R.string.pref_version));
            if (p != null) {
                p.setSummary(
                    getString(R.string.setting_version_desc,
                        info.versionName,
                        info.versionCode));
            }
        }
        catch (Exception ex) {
            Log.i("SettingsActivity", "failed to get version");
        }
    }
}
