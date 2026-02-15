package com.modmenu.loader;

import android.app.Application;
import android.content.Context;
import android.util.Log;
import java.lang.reflect.Method;

public class ModMenuLoader {
    private static final String TAG = "ModMenuLoader";
    private static boolean initialized = false;
    
    public static void init(Context context) {
        if (initialized) return;
        initialized = true;
        
        Log.d(TAG, "Initializing Mod Menu Loader...");
        
        try {
            System.loadLibrary("vis");
            
            nativeInit(context);
            
            OverlayService.start(context);
            
            Log.d(TAG, "Mod menu loaded successfully!");
        } catch (Exception e) {
            Log.e(TAG, "Failed to load mod menu", e);
        }
    }
    
    public static void inject(Application app) {
        try {
            Context context = app.getApplicationContext();
            init(context);
            
            app.registerActivityLifecycleCallbacks(new LifecycleListener());
            
        } catch (Exception e) {
            Log.e(TAG, "Injection failed", e);
        }
    }
    
    private static native void nativeInit(Context context);
    public static native void toggleMenu();
    public static native boolean isMenuVisible();
}
