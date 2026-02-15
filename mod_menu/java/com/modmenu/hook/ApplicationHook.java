package com.modmenu.hook;

import android.app.Application;
import android.content.Context;
import android.util.Log;
import com.modmenu.loader.ModLoader;

public class ApplicationHook extends Application {
    private static final String TAG = "ApplicationHook";
    
    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
        
        try {
            Log.d(TAG, "Application hook attached");
            
            // Initialize mod loader early
            ModLoader.init(base);
            
        } catch (Exception e) {
            Log.e(TAG, "Failed to attach hook", e);
        }
    }
    
    @Override
    public void onCreate() {
        super.onCreate();
        
        try {
            Log.d(TAG, "Application created");
            
            // Ensure initialization
            if (!ModLoader.isInitialized()) {
                ModLoader.init(getApplicationContext());
            }
            
        } catch (Exception e) {
            Log.e(TAG, "Failed in onCreate", e);
        }
    }
}
