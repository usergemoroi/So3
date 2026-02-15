package com.modmenu.overlay;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.os.Build;
import android.os.IBinder;
import android.provider.Settings;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

public class OverlayService extends Service {
    private WindowManager windowManager;
    private View overlayView;
    private View menuView;
    private boolean menuVisible = false;
    
    // Native methods
    static {
        System.loadLibrary("gamecore");
    }
    
    private native void nativeSetESPEnabled(boolean enabled);
    private native void nativeSetESPSkeleton(boolean enabled);
    private native void nativeSetESPBox(boolean enabled);
    private native void nativeSetESPDistance(boolean enabled);
    private native void nativeSetESPHealth(boolean enabled);
    private native void nativeSetESPName(boolean enabled);
    
    private native void nativeSetAimbotEnabled(boolean enabled);
    private native void nativeSetAimbotVisibleOnly(boolean enabled);
    private native void nativeSetAimbotTeamCheck(boolean enabled);
    private native void nativeSetAimbotFOV(float fov);
    private native void nativeSetAimbotSmooth(float smooth);
    
    private native void nativeSetProtectionEnabled(boolean enabled);
    private native void nativeSetAntiDebug(boolean enabled);
    private native void nativeSetHideModule(boolean enabled);
    private native void nativeSetRandomTiming(boolean enabled);
    
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
    
    @Override
    public void onCreate() {
        super.onCreate();
        
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (!Settings.canDrawOverlays(this)) {
                Toast.makeText(this, "Overlay permission required", Toast.LENGTH_LONG).show();
                stopSelf();
                return;
            }
        }
        
        windowManager = (WindowManager) getSystemService(WINDOW_SERVICE);
        createOverlay();
        createMenu();
    }
    
    private void createOverlay() {
        overlayView = new Button(this);
        ((Button) overlayView).setText("☰");
        ((Button) overlayView).setTextSize(20);
        
        int layoutType;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            layoutType = WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY;
        } else {
            layoutType = WindowManager.LayoutParams.TYPE_PHONE;
        }
        
        WindowManager.LayoutParams params = new WindowManager.LayoutParams(
            150,
            150,
            layoutType,
            WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
            PixelFormat.TRANSLUCENT
        );
        
        params.gravity = Gravity.TOP | Gravity.START;
        params.x = 20;
        params.y = 20;
        
        overlayView.setOnClickListener(v -> toggleMenu());
        
        windowManager.addView(overlayView, params);
    }
    
    private void createMenu() {
        LayoutInflater inflater = (LayoutInflater) getSystemService(LAYOUT_INFLATER_SERVICE);
        
        // Create menu programmatically
        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setPadding(20, 20, 20, 20);
        layout.setBackgroundColor(0xDD000000);
        
        // Title
        TextView title = new TextView(this);
        title.setText("Mod Menu v1.1");
        title.setTextColor(0xFFFFFFFF);
        title.setTextSize(18);
        title.setPadding(0, 0, 0, 20);
        layout.addView(title);
        
        // ESP Section
        TextView espTitle = new TextView(this);
        espTitle.setText("ESP");
        espTitle.setTextColor(0xFF00FF00);
        espTitle.setTextSize(16);
        layout.addView(espTitle);
        
        CheckBox espEnabled = createCheckBox("Enable ESP", false);
        espEnabled.setOnCheckedChangeListener((btn, checked) -> nativeSetESPEnabled(checked));
        layout.addView(espEnabled);
        
        CheckBox espSkeleton = createCheckBox("Skeleton", false);
        espSkeleton.setOnCheckedChangeListener((btn, checked) -> nativeSetESPSkeleton(checked));
        layout.addView(espSkeleton);
        
        CheckBox espBox = createCheckBox("Box", false);
        espBox.setOnCheckedChangeListener((btn, checked) -> nativeSetESPBox(checked));
        layout.addView(espBox);
        
        CheckBox espDistance = createCheckBox("Distance", false);
        espDistance.setOnCheckedChangeListener((btn, checked) -> nativeSetESPDistance(checked));
        layout.addView(espDistance);
        
        CheckBox espHealth = createCheckBox("Health", false);
        espHealth.setOnCheckedChangeListener((btn, checked) -> nativeSetESPHealth(checked));
        layout.addView(espHealth);
        
        CheckBox espName = createCheckBox("Name", false);
        espName.setOnCheckedChangeListener((btn, checked) -> nativeSetESPName(checked));
        layout.addView(espName);
        
        // Aimbot Section
        TextView aimbotTitle = new TextView(this);
        aimbotTitle.setText("\nAimbot");
        aimbotTitle.setTextColor(0xFF00FF00);
        aimbotTitle.setTextSize(16);
        layout.addView(aimbotTitle);
        
        CheckBox aimbotEnabled = createCheckBox("Enable Aimbot", false);
        aimbotEnabled.setOnCheckedChangeListener((btn, checked) -> nativeSetAimbotEnabled(checked));
        layout.addView(aimbotEnabled);
        
        CheckBox aimbotVisible = createCheckBox("Visible Only", true);
        aimbotVisible.setOnCheckedChangeListener((btn, checked) -> nativeSetAimbotVisibleOnly(checked));
        layout.addView(aimbotVisible);
        
        CheckBox aimbotTeam = createCheckBox("Team Check", true);
        aimbotTeam.setOnCheckedChangeListener((btn, checked) -> nativeSetAimbotTeamCheck(checked));
        layout.addView(aimbotTeam);
        
        // FOV Slider
        TextView fovLabel = new TextView(this);
        fovLabel.setText("FOV: 90°");
        fovLabel.setTextColor(0xFFFFFFFF);
        layout.addView(fovLabel);
        
        SeekBar fovSeekBar = new SeekBar(this);
        fovSeekBar.setMax(170);
        fovSeekBar.setProgress(80);
        fovSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                float fov = progress + 10;
                fovLabel.setText("FOV: " + (int)fov + "°");
                nativeSetAimbotFOV(fov);
            }
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });
        layout.addView(fovSeekBar);
        
        // Smooth Slider
        TextView smoothLabel = new TextView(this);
        smoothLabel.setText("Smooth: 5.0");
        smoothLabel.setTextColor(0xFFFFFFFF);
        layout.addView(smoothLabel);
        
        SeekBar smoothSeekBar = new SeekBar(this);
        smoothSeekBar.setMax(190);
        smoothSeekBar.setProgress(40);
        smoothSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                float smooth = (progress + 10) / 10.0f;
                smoothLabel.setText(String.format("Smooth: %.1f", smooth));
                nativeSetAimbotSmooth(smooth);
            }
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });
        layout.addView(smoothSeekBar);
        
        // Protection Section
        TextView protectionTitle = new TextView(this);
        protectionTitle.setText("\nProtection");
        protectionTitle.setTextColor(0xFF00FF00);
        protectionTitle.setTextSize(16);
        layout.addView(protectionTitle);
        
        CheckBox protectionEnabled = createCheckBox("Enable Protection", true);
        protectionEnabled.setOnCheckedChangeListener((btn, checked) -> nativeSetProtectionEnabled(checked));
        layout.addView(protectionEnabled);
        
        CheckBox antiDebug = createCheckBox("Anti Debug", true);
        antiDebug.setOnCheckedChangeListener((btn, checked) -> nativeSetAntiDebug(checked));
        layout.addView(antiDebug);
        
        CheckBox hideModule = createCheckBox("Hide Module", true);
        hideModule.setOnCheckedChangeListener((btn, checked) -> nativeSetHideModule(checked));
        layout.addView(hideModule);
        
        CheckBox randomTiming = createCheckBox("Random Timing", true);
        randomTiming.setOnCheckedChangeListener((btn, checked) -> nativeSetRandomTiming(checked));
        layout.addView(randomTiming);
        
        // Close button
        Button closeBtn = new Button(this);
        closeBtn.setText("Close Menu");
        closeBtn.setOnClickListener(v -> toggleMenu());
        layout.addView(closeBtn);
        
        menuView = layout;
    }
    
    private CheckBox createCheckBox(String text, boolean checked) {
        CheckBox cb = new CheckBox(this);
        cb.setText(text);
        cb.setTextColor(0xFFFFFFFF);
        cb.setChecked(checked);
        return cb;
    }
    
    private void toggleMenu() {
        if (menuVisible) {
            if (menuView.getParent() != null) {
                windowManager.removeView(menuView);
            }
            menuVisible = false;
        } else {
            int layoutType;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                layoutType = WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY;
            } else {
                layoutType = WindowManager.LayoutParams.TYPE_PHONE;
            }
            
            WindowManager.LayoutParams params = new WindowManager.LayoutParams(
                WindowManager.LayoutParams.WRAP_CONTENT,
                WindowManager.LayoutParams.WRAP_CONTENT,
                layoutType,
                WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL,
                PixelFormat.TRANSLUCENT
            );
            
            params.gravity = Gravity.CENTER;
            
            windowManager.addView(menuView, params);
            menuVisible = true;
        }
    }
    
    @Override
    public void onDestroy() {
        super.onDestroy();
        if (overlayView != null && overlayView.getParent() != null) {
            windowManager.removeView(overlayView);
        }
        if (menuView != null && menuView.getParent() != null) {
            windowManager.removeView(menuView);
        }
    }
}
