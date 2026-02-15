package a.b.c;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.os.Build;
import android.os.IBinder;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;

public class e extends Service {
    private WindowManager wm;
    private View mv;
    private View btn;
    private boolean vis = false;
    
    public static void start(Context ctx) {
        Intent i = new Intent(ctx, e.class);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            ctx.startForegroundService(i);
        } else {
            ctx.startService(i);
        }
    }
    
    @Override
    public void onCreate() {
        super.onCreate();
        wm = (WindowManager) getSystemService(WINDOW_SERVICE);
        createBtn();
        createMenu();
    }
    
    private void createBtn() {
        btn = new Button(this);
        ((Button) btn).setText("M");
        btn.setBackgroundColor(0x80000000);
        ((Button) btn).setTextColor(0xFFFFFFFF);
        
        int type = Build.VERSION.SDK_INT >= Build.VERSION_CODES.O 
            ? WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY
            : WindowManager.LayoutParams.TYPE_PHONE;
        
        WindowManager.LayoutParams p = new WindowManager.LayoutParams(
            WindowManager.LayoutParams.WRAP_CONTENT,
            WindowManager.LayoutParams.WRAP_CONTENT,
            type,
            WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
            PixelFormat.TRANSLUCENT
        );
        p.gravity = Gravity.TOP | Gravity.START;
        p.x = 50; p.y = 100;
        btn.setOnTouchListener(new DragListener(p));
        btn.setOnClickListener(v -> toggle());
        wm.addView(btn, p);
    }
    
    private void createMenu() {
        mv = buildMenu();
        int type = Build.VERSION.SDK_INT >= Build.VERSION_CODES.O 
            ? WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY
            : WindowManager.LayoutParams.TYPE_PHONE;
        WindowManager.LayoutParams p = new WindowManager.LayoutParams(
            800, 1000, type,
            WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
            PixelFormat.TRANSLUCENT
        );
        p.gravity = Gravity.CENTER;
        mv.setVisibility(View.GONE);
        wm.addView(mv, p);
    }
    
    private View buildMenu() {
        LinearLayout l = new LinearLayout(this);
        l.setOrientation(LinearLayout.VERTICAL);
        l.setBackgroundColor(0xE0000000);
        l.setPadding(40, 40, 40, 40);
        
        TextView t = new TextView(this);
        t.setText("S2M");
        t.setTextColor(0xFFFFFFFF);
        t.setTextSize(24);
        t.setGravity(Gravity.CENTER);
        l.addView(t);
        
        addLine(l);
        
        CheckBox esp = new CheckBox(this);
        esp.setText("E");
        esp.setTextColor(0xFFFFFFFF);
        esp.setOnCheckedChangeListener((v, c) -> j(c));
        l.addView(esp);
        
        CheckBox box = new CheckBox(this);
        box.setText("B");
        box.setTextColor(0xFFFFFFFF);
        box.setOnCheckedChangeListener((v, c) -> k(c));
        l.addView(box);
        
        CheckBox skel = new CheckBox(this);
        skel.setText("S");
        skel.setTextColor(0xFFFFFFFF);
        skel.setOnCheckedChangeListener((v, c) -> l(c));
        l.addView(skel);
        
        addLine(l);
        
        CheckBox aim = new CheckBox(this);
        aim.setText("A");
        aim.setTextColor(0xFFFFFFFF);
        aim.setOnCheckedChangeListener((v, c) -> n(c));
        l.addView(aim);
        
        SeekBar fov = new SeekBar(this);
        fov.setMax(170);
        fov.setProgress(80);
        fov.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            public void onProgressChanged(SeekBar s, int p, boolean f) { p(p + 10); }
            public void onStartTrackingTouch(SeekBar s) {}
            public void onStopTrackingTouch(SeekBar s) {}
        });
        l.addView(fov);
        
        Button close = new Button(this);
        close.setText("X");
        close.setOnClickListener(v -> toggle());
        l.addView(close);
        
        return l;
    }
    
    private void addLine(LinearLayout l) {
        View s = new View(this);
        s.setBackgroundColor(0xFF444444);
        LinearLayout.LayoutParams p = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, 2);
        p.setMargins(0, 20, 0, 20);
        s.setLayoutParams(p);
        l.addView(s);
    }
    
    private void toggle() {
        vis = !vis;
        mv.setVisibility(vis ? View.VISIBLE : View.GONE);
    }
    
    private class DragListener implements View.OnTouchListener {
        private WindowManager.LayoutParams p;
        private int ix, iy;
        private float tx, ty;
        
        DragListener(WindowManager.LayoutParams params) { this.p = params; }
        
        @Override
        public boolean onTouch(View v, MotionEvent e) {
            switch (e.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    ix = p.x; iy = p.y;
                    tx = e.getRawX(); ty = e.getRawY();
                    return false;
                case MotionEvent.ACTION_MOVE:
                    p.x = ix + (int)(e.getRawX() - tx);
                    p.y = iy + (int)(e.getRawY() - ty);
                    wm.updateViewLayout(v, p);
                    return false;
            }
            return false;
        }
    }
    
    @Override
    public IBinder onBind(Intent i) { return null; }
    
    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mv != null) wm.removeView(mv);
        if (btn != null) wm.removeView(btn);
    }
    
    // Native methods mapping: j=ESP, k=Box, l=Skeleton, m=Health, n=Aimbot, o=Visible, p=FOV, q=Smooth
    private native void j(boolean e);
    private native void k(boolean e);
    private native void l(boolean e);
    private native void m(boolean e);
    private native void n(boolean e);
    private native void o(boolean e);
    private native void p(float v);
    private native void q(float v);
}
