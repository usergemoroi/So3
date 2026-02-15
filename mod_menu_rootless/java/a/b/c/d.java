package a.b.c;

import android.app.Application;
import android.content.Context;

public class d {
    private static boolean initialized = false;
    
    public static void init(Context context) {
        if (initialized) return;
        initialized = true;
        
        try {
            System.loadLibrary("v");
            e(context);
            e.start(context);
        } catch (Exception ex) {}
    }
    
    public static void inject(Application app) {
        try {
            init(app.getApplicationContext());
            app.registerActivityLifecycleCallbacks(new g());
        } catch (Exception ex) {}
    }
    
    private static native void e(Context context);
    public static native void f();
    public static native boolean g();
}
