package com.modmenu.loader;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.os.Build;
import android.os.IBinder;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.CheckBox;
import android.widget.SeekBar;
import android.widget.TextView;
import android.util.Log;

public class OverlayService extends Service {
    private static final String TAG = "OverlayService";
    private WindowManager windowManager;
    private View overlayView;
    private View menuButton;
    private boolean menuVisible = false;
    
    public static void start(Context context) {
        Intent intent = new Intent(context, OverlayService.class);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            context.startForegroundService(intent);
        } else {
            context.startService(intent);
        }
    }
    
    @Override
    public void onCreate() {
        super.onCreate();
        
        windowManager = (WindowManager) getSystemService(WINDOW_SERVICE);
        
        createMenuButton();
        createOverlayMenu();
    }
    
    private void createMenuButton() {
        menuButton = new Button(this);
        ((Button) menuButton).setText("Menu");
        menuButton.setBackgroundColor(0x80000000);
        ((Button) menuButton).setTextColor(0xFFFFFFFF);
        
        int layoutType = Build.VERSION.SDK_INT >= Build.VERSION_CODES.O 
            ? WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY
            : WindowManager.LayoutParams.TYPE_PHONE;
        
        WindowManager.LayoutParams params = new WindowManager.LayoutParams(
            WindowManager.LayoutParams.WRAP_CONTENT,
            WindowManager.LayoutParams.WRAP_CONTENT,
            layoutType,
            WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
            PixelFormat.TRANSLUCENT
        );
        
        params.gravity = Gravity.TOP | Gravity.START;
        params.x = 50;
        params.y = 100;
        
        menuButton.setOnTouchListener(new DragTouchListener(params));
        menuButton.setOnClickListener(v -> toggleMenu());
        
        windowManager.addView(menuButton, params);
    }
    
    private void createOverlayMenu() {
        overlayView = createMenuLayout();
        
        int layoutType = Build.VERSION.SDK_INT >= Build.VERSION_CODES.O 
            ? WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY
            : WindowManager.LayoutParams.TYPE_PHONE;
        
        WindowManager.LayoutParams params = new WindowManager.LayoutParams(
            800,
            1000,
            layoutType,
            WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
            PixelFormat.TRANSLUCENT
        );
        
        params.gravity = Gravity.CENTER;
        
        overlayView.setVisibility(View.GONE);
        windowManager.addView(overlayView, params);
    }
    
    private View createMenuLayout() {
        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setBackgroundColor(0xE0000000);
        layout.setPadding(40, 40, 40, 40);
        
        TextView title = new TextView(this);
        title.setText("Standoff 2 Mod Menu");
        title.setTextColor(0xFFFFFFFF);
        title.setTextSize(24);
        title.setGravity(Gravity.CENTER);
        layout.addView(title);
        
        addSeparator(layout);
        
        TextView espHeader = new TextView(this);
        espHeader.setText("ESP Settings");
        espHeader.setTextColor(0xFF00FF00);
        espHeader.setTextSize(20);
        layout.addView(espHeader);
        
        CheckBox espEnabled = new CheckBox(this);
        espEnabled.setText("Enable ESP");
        espEnabled.setTextColor(0xFFFFFFFF);
        espEnabled.setOnCheckedChangeListener((v, checked) -> 
            nativeSetESPEnabled(checked));
        layout.addView(espEnabled);
        
        CheckBox espBox = new CheckBox(this);
        espBox.setText("Box ESP");
        espBox.setTextColor(0xFFFFFFFF);
        espBox.setOnCheckedChangeListener((v, checked) -> 
            nativeSetESPBox(checked));
        layout.addView(espBox);
        
        CheckBox espSkeleton = new CheckBox(this);
        espSkeleton.setText("Skeleton ESP");
        espSkeleton.setTextColor(0xFFFFFFFF);
        espSkeleton.setOnCheckedChangeListener((v, checked) -> 
            nativeSetESPSkeleton(checked));
        layout.addView(espSkeleton);
        
        CheckBox espHealth = new CheckBox(this);
        espHealth.setText("Health ESP");
        espHealth.setTextColor(0xFFFFFFFF);
        espHealth.setOnCheckedChangeListener((v, checked) -> 
            nativeSetESPHealth(checked));
        layout.addView(espHealth);
        
        addSeparator(layout);
        
        TextView aimbotHeader = new TextView(this);
        aimbotHeader.setText("Aimbot Settings");
        aimbotHeader.setTextColor(0xFF00FF00);
        aimbotHeader.setTextSize(20);
        layout.addView(aimbotHeader);
        
        CheckBox aimbotEnabled = new CheckBox(this);
        aimbotEnabled.setText("Enable Aimbot");
        aimbotEnabled.setTextColor(0xFFFFFFFF);
        aimbotEnabled.setOnCheckedChangeListener((v, checked) -> 
            nativeSetAimbotEnabled(checked));
        layout.addView(aimbotEnabled);
        
        CheckBox aimbotVisible = new CheckBox(this);
        aimbotVisible.setText("Visible Only");
        aimbotVisible.setTextColor(0xFFFFFFFF);
        aimbotVisible.setChecked(true);
        aimbotVisible.setOnCheckedChangeListener((v, checked) -> 
            nativeSetAimbotVisibleOnly(checked));
        layout.addView(aimbotVisible);
        
        TextView fovLabel = new TextView(this);
        fovLabel.setText("FOV: 90");
        fovLabel.setTextColor(0xFFFFFFFF);
        layout.addView(fovLabel);
        
        SeekBar fovSeek = new SeekBar(this);
        fovSeek.setMax(170);
        fovSeek.setProgress(80);
        fovSeek.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                int fov = progress + 10;
                fovLabel.setText("FOV: " + fov);
                nativeSetAimbotFOV(fov);
            }
            public void onStartTrackingTouch(SeekBar seekBar) {}
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });
        layout.addView(fovSeek);
        
        TextView smoothLabel = new TextView(this);
        smoothLabel.setText("Smooth: 5.0");
        smoothLabel.setTextColor(0xFFFFFFFF);
        layout.addView(smoothLabel);
        
        SeekBar smoothSeek = new SeekBar(this);
        smoothSeek.setMax(190);
        smoothSeek.setProgress(40);
        smoothSeek.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                float smooth = (progress + 10) / 10.0f;
                smoothLabel.setText(String.format("Smooth: %.1f", smooth));
                nativeSetAimbotSmooth(smooth);
            }
            public void onStartTrackingTouch(SeekBar seekBar) {}
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });
        layout.addView(smoothSeek);
        
        addSeparator(layout);
        
        Button closeButton = new Button(this);
        closeButton.setText("Close Menu");
        closeButton.setOnClickListener(v -> toggleMenu());
        layout.addView(closeButton);
        
        return layout;
    }
    
    private void addSeparator(LinearLayout layout) {
        View separator = new View(this);
        separator.setBackgroundColor(0xFF444444);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, 2);
        params.setMargins(0, 20, 0, 20);
        separator.setLayoutParams(params);
        layout.addView(separator);
    }
    
    private void toggleMenu() {
        menuVisible = !menuVisible;
        overlayView.setVisibility(menuVisible ? View.VISIBLE : View.GONE);
    }
    
    private class DragTouchListener implements View.OnTouchListener {
        private WindowManager.LayoutParams params;
        private int initialX, initialY;
        private float initialTouchX, initialTouchY;
        
        DragTouchListener(WindowManager.LayoutParams params) {
            this.params = params;
        }
        
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    initialX = params.x;
                    initialY = params.y;
                    initialTouchX = event.getRawX();
                    initialTouchY = event.getRawY();
                    return false;
                    
                case MotionEvent.ACTION_MOVE:
                    params.x = initialX + (int) (event.getRawX() - initialTouchX);
                    params.y = initialY + (int) (event.getRawY() - initialTouchY);
                    windowManager.updateViewLayout(v, params);
                    return false;
            }
            return false;
        }
    }
    
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
    
    @Override
    public void onDestroy() {
        super.onDestroy();
        if (overlayView != null) windowManager.removeView(overlayView);
        if (menuButton != null) windowManager.removeView(menuButton);
    }
    
    private native void nativeSetESPEnabled(boolean enabled);
    private native void nativeSetESPBox(boolean enabled);
    private native void nativeSetESPSkeleton(boolean enabled);
    private native void nativeSetESPHealth(boolean enabled);
    private native void nativeSetAimbotEnabled(boolean enabled);
    private native void nativeSetAimbotVisibleOnly(boolean enabled);
    private native void nativeSetAimbotFOV(float fov);
    private native void nativeSetAimbotSmooth(float smooth);
}
