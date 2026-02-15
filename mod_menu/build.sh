#!/bin/bash

echo "=========================================="
echo "  Standoff 2 Mod Menu - Build Script"
echo "=========================================="
echo ""

if [ -z "$ANDROID_NDK_HOME" ]; then
    echo "ERROR: ANDROID_NDK_HOME not set!"
    echo "Please set ANDROID_NDK_HOME environment variable"
    echo "Example: export ANDROID_NDK_HOME=/path/to/ndk"
    exit 1
fi

echo "Using NDK: $ANDROID_NDK_HOME"
echo ""

cd jni

echo "Building mod menu..."
$ANDROID_NDK_HOME/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk

if [ $? -eq 0 ]; then
    echo ""
    echo "=========================================="
    echo "  Build successful!"
    echo "=========================================="
    echo ""
    echo "Output library:"
    echo "  ../libs/arm64-v8a/libmodmenu.so"
    echo ""
else
    echo ""
    echo "=========================================="
    echo "  Build failed!"
    echo "=========================================="
    exit 1
fi
