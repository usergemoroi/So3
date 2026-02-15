#!/bin/bash

echo "=========================================="
echo "  Rootless Mod Menu - Quick Installer"
echo "=========================================="
echo ""

if [ -z "$1" ]; then
    echo "Usage: $0 <path_to_standoff2.apk>"
    exit 1
fi

APK_PATH="$1"

if [ ! -f "$APK_PATH" ]; then
    echo "Error: APK not found: $APK_PATH"
    exit 1
fi

echo "[1/5] Building mod..."
cd ..
./build.sh
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo ""
echo "[2/5] Patching APK..."
cd tools
python3 apk_patcher.py "$APK_PATH"
if [ $? -ne 0 ]; then
    echo "Patching failed!"
    exit 1
fi

MODDED_APK=$(ls -t *_modded_signed.apk 2>/dev/null | head -1)

if [ -z "$MODDED_APK" ]; then
    echo "Error: Modded APK not found!"
    exit 1
fi

echo ""
echo "[3/5] Checking device connection..."
adb devices | grep -q "device$"
if [ $? -ne 0 ]; then
    echo "No device connected!"
    echo "Please connect your device and enable USB debugging"
    exit 1
fi

echo ""
echo "[4/5] Uninstalling old version..."
PACKAGE=$(aapt dump badging "$APK_PATH" | grep package | awk '{print $2}' | sed s/name=//g | sed s/\'//g)
adb uninstall "$PACKAGE" 2>/dev/null

echo ""
echo "[5/5] Installing modded APK..."
adb install "$MODDED_APK"

if [ $? -eq 0 ]; then
    echo ""
    echo "=========================================="
    echo "  Installation successful!"
    echo "=========================================="
    echo ""
    echo "Next steps:"
    echo "  1. Open Standoff 2 on your device"
    echo "  2. Allow 'Display over other apps' permission"
    echo "  3. Look for 'Menu' button in game"
    echo ""
    echo "Happy gaming! 🎮"
else
    echo ""
    echo "Installation failed!"
    echo "Try manual installation: adb install $MODDED_APK"
fi
