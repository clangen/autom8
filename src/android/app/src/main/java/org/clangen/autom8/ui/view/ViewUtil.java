package org.clangen.autom8.ui.view;

import android.animation.Animator;
import android.os.Build;
import android.view.View;
import android.view.ViewAnimationUtils;

public final class ViewUtil {
    private static final int DEFAULT_DURATION = 500;

    public static void performCircularAnimationIn(final View view) {
        performCircularAnimationIn(view, DEFAULT_DURATION);
    }

    public static void performCircularAnimationIn(final View view, int duration) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            final int centerX = view.getWidth();
            final int centerY = 0;
            final float radius = (float) Math.hypot(view.getWidth(), view.getHeight());
            final Animator anim = ViewAnimationUtils.createCircularReveal(view, centerX, centerY, 0, radius);
            anim.setDuration(duration);
            view.setVisibility(View.VISIBLE);
            anim.start();
        }
    }
}
