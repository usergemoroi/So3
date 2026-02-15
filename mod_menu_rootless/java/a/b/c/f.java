package a.b.c;

import android.content.Context;
import dalvik.system.DexClassLoader;
import dalvik.system.PathClassLoader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.lang.reflect.Array;
import java.lang.reflect.Field;

public class f {
    public static void inject(Context ctx) {
        try {
            File dex = extract(ctx);
            ClassLoader scl = ClassLoader.getSystemClassLoader();
            Class<?> bdcl = Class.forName("dalvik.system.BaseDexClassLoader");
            Field plf = bdcl.getDeclaredField("pathList");
            plf.setAccessible(true);
            Object spl = plf.get(scl);
            File out = ctx.getDir("dex", Context.MODE_PRIVATE);
            DexClassLoader dcl = new DexClassLoader(
                dex.getAbsolutePath(), out.getAbsolutePath(), null, scl);
            Object npl = plf.get(dcl);
            Class<?> plc = Class.forName("dalvik.system.DexPathList");
            Field def = plc.getDeclaredField("dexElements");
            def.setAccessible(true);
            Object sde = def.get(spl);
            Object nde = def.get(npl);
            Object comb = combine(sde, nde);
            def.set(spl, comb);
            Class<?> lc = dcl.loadClass("a.b.c.d");
            lc.getMethod("init", Context.class).invoke(null, ctx);
        } catch (Exception ex) {}
    }
    
    private static File extract(Context ctx) throws Exception {
        File f = new File(ctx.getFilesDir(), "a");
        if (!f.exists()) {
            InputStream i = ctx.getAssets().open("a");  // was modmenu.dex
            FileOutputStream o = new FileOutputStream(f);
            byte[] b = new byte[8192];
            int r;
            while ((r = i.read(b)) != -1) o.write(b, 0, r);
            o.close(); i.close();
        }
        return f;
    }
    
    private static Object combine(Object a1, Object a2) {
        Class<?> ct = a1.getClass().getComponentType();
        int l1 = Array.getLength(a1);
        int l2 = Array.getLength(a2);
        Object na = Array.newInstance(ct, l1 + l2);
        for (int i = 0; i < l1; i++) Array.set(na, i, Array.get(a1, i));
        for (int i = 0; i < l2; i++) Array.set(na, l1 + i, Array.get(a2, i));
        return na;
    }
}
