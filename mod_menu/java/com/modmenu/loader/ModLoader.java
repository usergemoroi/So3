package com.modmenu.loader;

import android.app.Application;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.provider.Settings;
import android.util.Log;
import java.lang.reflect.Method;

public class ModLoader {
    private static final String TAG = "ModLoader";
    private static boolean initialized = false;
    
    // Load native library
    static {
        try {
            System.loadLibrary("gamecore");
            Log.d(TAG, "Native library loaded successfully");
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "Failed to load native library", e);
        }
    }
    
    // Native initialization
    private static native void nativeInit(Context context);
    private static native void nativeStart();
    
    public static void init(Context context) {
        if (initialized) {
            return;
        }
        
        try {
            Log.d(TAG, "Initializing ModLoader...");
            
            // Initialize native layer
            nativeInit(context);
            
            // Check and request overlay permission
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                if (!Settings.canDrawOverlays(context)) {
                    Intent intent = new Intent(Settings.ACTION_MANAGE_OVERLAY_PERMISSION);
                    intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    context.startActivity(intent);
                    Log.w(TAG, "Overlay permission not granted, requesting...");
                } else {
                    startOverlayService(context);
                }
            } else {
                startOverlayService(context);
            }
            
            // Start native hooks
            nativeStart();
            
            initialized = true;
            Log.d(TAG, "ModLoader initialized successfully");
            
        } catch (Exception e) {
            Log.e(TAG, "Failed to initialize ModLoader", e);
        }
    }
    
    private static void startOverlayService(Context context) {
        try {
            Intent serviceIntent = new Intent(context, com.modmenu.overlay.OverlayService.class);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                context.startForegroundService(serviceIntent);
            } else {
                context.startService(serviceIntent);
            }
            Log.d(TAG, "Overlay service started");
        } catch (Exception e) {
            Log.e(TAG, "Failed to start overlay service", e);
        }
    }
    
    public static boolean isInitialized() {
        return initialized;
    }
}
