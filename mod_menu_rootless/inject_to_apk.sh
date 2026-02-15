#!/bin/bash

set -e

echo "=========================================="
echo "  Injecting classes.dex and libvis.so"
echo "  into extracted APK"
echo "=========================================="
echo ""

PROJECT_ROOT="/home/engine/project"
MOD_DIR="$PROJECT_ROOT/mod_menu_rootless"
OUTPUT_DIR="$MOD_DIR/output"

# Проверка наличия собранных файлов
if [ ! -f "$OUTPUT_DIR/classes.dex" ]; then
    echo "ERROR: classes.dex not found!"
    echo "Please run build_vis.sh first"
    exit 1
fi

if [ ! -f "$OUTPUT_DIR/libvis.so.arm64-v8a" ] && [ ! -f "$OUTPUT_DIR/libvis.so.armeabi-v7a" ]; then
    echo "ERROR: libvis.so not found!"
    echo "Please run build_vis.sh first"
    exit 1
fi

echo "[1/5] Copying classes.dex to APK assets..."
mkdir -p "$PROJECT_ROOT/assets"
cp "$OUTPUT_DIR/classes.dex" "$PROJECT_ROOT/assets/classes.dex"
echo "✓ Copied to $PROJECT_ROOT/assets/classes.dex"
echo ""

echo "[2/5] Copying libvis.so to APK lib directories..."

# ARM64
if [ -f "$OUTPUT_DIR/libvis.so.arm64-v8a" ]; then
    mkdir -p "$PROJECT_ROOT/lib/arm64-v8a"
    cp "$OUTPUT_DIR/libvis.so.arm64-v8a" "$PROJECT_ROOT/lib/arm64-v8a/libvis.so"
    echo "✓ Copied to $PROJECT_ROOT/lib/arm64-v8a/libvis.so"
fi

# ARM32
if [ -f "$OUTPUT_DIR/libvis.so.armeabi-v7a" ]; then
    mkdir -p "$PROJECT_ROOT/lib/armeabi-v7a"
    cp "$OUTPUT_DIR/libvis.so.armeabi-v7a" "$PROJECT_ROOT/lib/armeabi-v7a/libvis.so"
    echo "✓ Copied to $PROJECT_ROOT/lib/armeabi-v7a/libvis.so"
fi

echo ""

echo "[3/5] Checking AndroidManifest.xml..."
if [ -f "$PROJECT_ROOT/AndroidManifest.xml" ]; then
    echo "✓ AndroidManifest.xml found"

    # Проверяем наличие нужных разрешений
    if ! grep -q "SYSTEM_ALERT_WINDOW" "$PROJECT_ROOT/AndroidManifest.xml"; then
        echo "! WARNING: SYSTEM_ALERT_WINDOW permission not found"
        echo "  You may need to add: <uses-permission android:name=\"android.permission.SYSTEM_ALERT_WINDOW\"/>"
    fi

    if ! grep -q "INTERNET" "$PROJECT_ROOT/AndroidManifest.xml"; then
        echo "! WARNING: INTERNET permission not found"
        echo "  You may need to add: <uses-permission android:name=\"android.permission.INTERNET\"/>"
    fi
else
    echo "! WARNING: AndroidManifest.xml not found"
fi
echo ""

echo "[4/5] Creating injection summary..."
cat > "$PROJECT_ROOT/INJECTION_INFO.txt" << 'EOF'
========================================
MOD MENU INJECTION INFO
========================================

Injected files:
- assets/classes.dex (Java mod menu loader)
- lib/arm64-v8a/libvis.so (Native mod menu library)
- lib/armeabi-v7a/libvis.so (Native mod menu library, if built)

Package: com.modmenu.loader

Required Permissions:
- android.permission.SYSTEM_ALERT_WINDOW
- android.permission.INTERNET

Required Service:
- com.modmenu.loader.OverlayService

To integrate into the app:
1. Find the main Application class in smali (smali/.../Application.smali)
2. In onCreate() method, add after invoke-super:
   invoke-static {p0}, Lcom/modmenu/loader/DexInjector;->inject(Landroid/content/Context;)V
   invoke-static {p0}, Lcom/modmenu/loader/ModMenuLoader;->inject(Landroid/app/Application;)V

3. Rebuild APK with apktool
4. Sign the APK
EOF

cat "$PROJECT_ROOT/INJECTION_INFO.txt"
echo ""
echo "✓ Created injection info at $PROJECT_ROOT/INJECTION_INFO.txt"
echo ""

echo "[5/5] Checking DEX files..."
echo "Found DEX files:"
ls -lh "$PROJECT_ROOT"/classes*.dex | awk '{print "  • " $9 " - " $5}'
echo ""

echo "=========================================="
echo "  Injection complete!"
echo "=========================================="
echo ""
echo "Files added to extracted APK:"
echo "  • assets/classes.dex"
echo "  • lib/arm64-v8a/libvis.so"
if [ -f "$PROJECT_ROOT/lib/armeabi-v7a/libvis.so" ]; then
    echo "  • lib/armeabi-v7a/libvis.so"
fi
echo ""
echo "Next steps to rebuild APK:"
echo "  1. Use apktool to rebuild: apktool b . -o output.apk"
echo "  2. Sign the APK: apksigner sign output.apk"
echo "  3. Install: adb install output.apk"
echo ""
