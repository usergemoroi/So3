package com.modmenu.loader;

import android.app.Activity;
import android.app.Application;
import android.os.Bundle;
import android.util.Log;

public class LifecycleListener implements Application.ActivityLifecycleCallbacks {
    private static final String TAG = "LifecycleListener";
    
    @Override
    public void onActivityCreated(Activity activity, Bundle savedInstanceState) {
        Log.d(TAG, "Activity created: " + activity.getClass().getName());
        nativeOnActivityCreated(activity);
    }
    
    @Override
    public void onActivityStarted(Activity activity) {
        nativeOnActivityStarted(activity);
    }
    
    @Override
    public void onActivityResumed(Activity activity) {
        nativeOnActivityResumed(activity);
    }
    
    @Override
    public void onActivityPaused(Activity activity) {
        nativeOnActivityPaused(activity);
    }
    
    @Override
    public void onActivityStopped(Activity activity) {
        nativeOnActivityStopped(activity);
    }
    
    @Override
    public void onActivitySaveInstanceState(Activity activity, Bundle outState) {
    }
    
    @Override
    public void onActivityDestroyed(Activity activity) {
        nativeOnActivityDestroyed(activity);
    }
    
    private native void nativeOnActivityCreated(Activity activity);
    private native void nativeOnActivityStarted(Activity activity);
    private native void nativeOnActivityResumed(Activity activity);
    private native void nativeOnActivityPaused(Activity activity);
    private native void nativeOnActivityStopped(Activity activity);
    private native void nativeOnActivityDestroyed(Activity activity);
}
