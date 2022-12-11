package org.clangen.autom8.device;

import android.content.Context;

import org.clangen.autom8.R;

import java.util.Comparator;

public class GroupComparator implements Comparator<Group> {
    private String KEY_SECURITY_ALERTS = "00";
    private String KEY_NO_SECURITY_ALERTS = "01";
    private String KEY_IS_VALID_GROUP = "00";
    private String KEY_IS_INVALID_GROUP = "01";

    private String NO_GROUPS_NAME;

    public GroupComparator(Context context) {
        NO_GROUPS_NAME = context.getString(R.string.device_group_no_groups);
    }

    @Override
    public int compare(Group g1, Group g2) {
        return sortKey(g1).compareTo(sortKey(g2));
    }

    private String sortKey(Group group) {
        final String groupSort =
            group.getName().equals(NO_GROUPS_NAME) ?
                KEY_IS_INVALID_GROUP :
                KEY_IS_VALID_GROUP;

        final String alertSort =
             group.getAlertCount() > 0 ?
                 KEY_SECURITY_ALERTS :
                 KEY_NO_SECURITY_ALERTS;

        return alertSort + "-" + groupSort + "-" + group.getName();
    }
}
