#!/bin/bash

set -e

echo "=========================================="
echo "  Rootless Mod Menu - Simple Build"
echo "=========================================="
echo ""

if [ -z "$ANDROID_NDK_HOME" ]; then
    echo "ERROR: ANDROID_NDK_HOME not set!"
    echo "Please set ANDROID_NDK_HOME environment variable"
    exit 1
fi

echo "[1/2] Building native library..."
cd jni
$ANDROID_NDK_HOME/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk -j4
cd ..

echo ""
echo "[2/2] Compiling Java classes..."
mkdir -p build/classes

# Compile Java files using the local Android SDK
javac -d build/classes \
    java/com/modmenu/loader/*.java

echo ""
echo "=========================================="
echo "  Build complete!"
echo "=========================================="
echo ""
echo "Output files:"
echo "  libs/arm64-v8a/libmodmenu_noroot.so"
echo "  libs/armeabi-v7a/libmodmenu_noroot.so (if built)"
echo "  build/classes/com/modmenu/loader/*.class"
echo ""
echo "To create DEX file:"
echo "  d8 --output build/ build/classes/com/modmenu/loader/*.class"
echo ""