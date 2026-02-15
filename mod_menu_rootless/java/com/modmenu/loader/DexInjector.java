package com.modmenu.loader;

import android.content.Context;
import android.util.Log;
import dalvik.system.DexClassLoader;
import dalvik.system.PathClassLoader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.lang.reflect.Array;
import java.lang.reflect.Field;

public class DexInjector {
    private static final String TAG = "DexInjector";
    
    public static void inject(Context context) {
        try {
            File dexFile = extractDex(context);
            
            ClassLoader systemClassLoader = ClassLoader.getSystemClassLoader();
            Class<?> baseDexClassLoaderClass = Class.forName("dalvik.system.BaseDexClassLoader");
            Field pathListField = baseDexClassLoaderClass.getDeclaredField("pathList");
            pathListField.setAccessible(true);
            
            Object systemPathList = pathListField.get(systemClassLoader);
            
            File dexOutputDir = context.getDir("dex", Context.MODE_PRIVATE);
            DexClassLoader dexClassLoader = new DexClassLoader(
                dexFile.getAbsolutePath(),
                dexOutputDir.getAbsolutePath(),
                null,
                systemClassLoader
            );
            
            Object newPathList = pathListField.get(dexClassLoader);
            
            Class<?> pathListClass = Class.forName("dalvik.system.DexPathList");
            Field dexElementsField = pathListClass.getDeclaredField("dexElements");
            dexElementsField.setAccessible(true);
            
            Object systemDexElements = dexElementsField.get(systemPathList);
            Object newDexElements = dexElementsField.get(newPathList);
            
            Object combined = combineArrays(systemDexElements, newDexElements);
            dexElementsField.set(systemPathList, combined);
            
            Log.d(TAG, "DEX injected successfully!");
            
            Class<?> loaderClass = dexClassLoader.loadClass("com.modmenu.loader.ModMenuLoader");
            loaderClass.getMethod("init", Context.class).invoke(null, context);
            
        } catch (Exception e) {
            Log.e(TAG, "DEX injection failed", e);
        }
    }
    
    private static File extractDex(Context context) throws Exception {
        File dexFile = new File(context.getFilesDir(), "modmenu.dex");
        
        if (!dexFile.exists()) {
            InputStream input = context.getAssets().open("modmenu.dex");
            FileOutputStream output = new FileOutputStream(dexFile);
            
            byte[] buffer = new byte[8192];
            int bytesRead;
            while ((bytesRead = input.read(buffer)) != -1) {
                output.write(buffer, 0, bytesRead);
            }
            
            output.close();
            input.close();
        }
        
        return dexFile;
    }
    
    private static Object combineArrays(Object array1, Object array2) {
        Class<?> componentType = array1.getClass().getComponentType();
        int length1 = Array.getLength(array1);
        int length2 = Array.getLength(array2);
        
        Object newArray = Array.newInstance(componentType, length1 + length2);
        
        for (int i = 0; i < length1; i++) {
            Array.set(newArray, i, Array.get(array1, i));
        }
        
        for (int i = 0; i < length2; i++) {
            Array.set(newArray, length1 + i, Array.get(array2, i));
        }
        
        return newArray;
    }
}
