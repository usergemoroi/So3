#!/bin/bash

set -e

echo "=========================================="
echo "  Rootless Mod Menu - Build Script"
echo "=========================================="
echo ""

if [ -z "$ANDROID_NDK_HOME" ]; then
    echo "ERROR: ANDROID_NDK_HOME not set!"
    echo "Please set ANDROID_NDK_HOME environment variable"
    exit 1
fi

echo "[1/4] Building native library..."
cd jni
$ANDROID_NDK_HOME/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk
cd ..

echo ""
echo "[2/4] Compiling Java classes..."
mkdir -p build/classes

javac -d build/classes \
    -source 1.8 -target 1.8 \
    -bootclasspath $ANDROID_HOME/platforms/android-28/android.jar \
    java/com/modmenu/loader/*.java

echo ""
echo "[3/4] Creating DEX file..."
$ANDROID_HOME/build-tools/*/d8 \
    --lib $ANDROID_HOME/platforms/android-28/android.jar \
    --release \
    --output build/ \
    build/classes/com/modmenu/loader/*.class

mv build/classes.dex build/modmenu.dex

echo ""
echo "[4/4] Packaging..."
mkdir -p output
cp libs/arm64-v8a/libmodmenu_noroot.so output/
cp libs/armeabi-v7a/libmodmenu_noroot.so output/ 2>/dev/null || true
cp build/modmenu.dex output/

echo ""
echo "=========================================="
echo "  Build complete!"
echo "=========================================="
echo ""
echo "Output files:"
echo "  output/libmodmenu_noroot.so (ARM64)"
echo "  output/libmodmenu_noroot.so (ARM32)"
echo "  output/modmenu.dex"
echo ""
echo "Integration instructions:"
echo "  1. Copy modmenu.dex to /assets/modmenu.dex in APK"
echo "  2. Copy libmodmenu_noroot.so to /lib/[arch]/ in APK"
echo "  3. Add overlay permission to AndroidManifest.xml"
echo "  4. Add loading code to main Application class"
echo ""
