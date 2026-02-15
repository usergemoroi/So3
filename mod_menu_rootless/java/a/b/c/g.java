package a.b.c;

import android.app.Activity;
import android.app.Application;
import android.os.Bundle;

public class g implements Application.ActivityLifecycleCallbacks {
    @Override
    public void onActivityCreated(Activity a, Bundle s) { t(a); }
    @Override
    public void onActivityStarted(Activity a) { u(a); }
    @Override
    public void onActivityResumed(Activity a) { v(a); }
    @Override
    public void onActivityPaused(Activity a) { w(a); }
    @Override
    public void onActivityStopped(Activity a) { x(a); }
    @Override
    public void onActivitySaveInstanceState(Activity a, Bundle s) {}
    @Override
    public void onActivityDestroyed(Activity a) { y(a); }
    
    private native void t(Activity a);
    private native void u(Activity a);
    private native void v(Activity a);
    private native void w(Activity a);
    private native void x(Activity a);
    private native void y(Activity a);
}
