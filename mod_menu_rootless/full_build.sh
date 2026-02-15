#!/bin/bash
set -e

echo "=========================================="
echo "  Complete Build & Inject Workflow"
echo "=========================================="
echo ""

if [ -z "$1" ]; then
    echo "Usage: $0 <path_to_standoff2.apk>"
    echo ""
    echo "Prerequisites:"
    echo "  export ANDROID_NDK_HOME=/path/to/ndk"
    echo "  export ANDROID_HOME=/path/to/sdk"
    echo ""
    exit 1
fi

APK_PATH="$1"

if [ ! -f "$APK_PATH" ]; then
    echo "ERROR: APK not found: $APK_PATH"
    exit 1
fi

# Step 1: Build
echo "=== STEP 1: Building mod ==="
./build.sh

if [ ! -d "output/lib" ] || [ ! -f "output/a.dex" ]; then
    echo "ERROR: Build failed - output files not found"
    exit 1
fi

echo ""
echo "=== STEP 2: Injecting into APK ==="
./inject_apk.sh "$APK_PATH"

echo ""
echo "=========================================="
echo "  All Done!"
echo "=========================================="
echo ""
echo "Install the modded APK:"
echo "  adb install -r standoff2_modded_signed.apk"
echo ""
echo "Then open Standoff 2 and grant overlay permission"
echo ""
