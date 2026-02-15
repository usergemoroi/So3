#!/bin/bash

echo "=========================================="
echo "  Device Compatibility Test"
echo "=========================================="
echo ""

echo "Checking device connection..."
adb devices -l

echo ""
echo "Device information:"
echo "  Model: $(adb shell getprop ro.product.model)"
echo "  Brand: $(adb shell getprop ro.product.brand)"
echo "  Android Version: $(adb shell getprop ro.build.version.release)"
echo "  SDK Level: $(adb shell getprop ro.build.version.sdk)"
echo "  Architecture: $(adb shell getprop ro.product.cpu.abi)"

SDK_LEVEL=$(adb shell getprop ro.build.version.sdk)
ARCH=$(adb shell getprop ro.product.cpu.abi)

echo ""
echo "Compatibility check:"

if [ "$SDK_LEVEL" -lt 24 ]; then
    echo "  ❌ Android version too old (minimum Android 7.0 / API 24)"
    echo "     Your device: API $SDK_LEVEL"
    COMPATIBLE=false
else
    echo "  ✅ Android version compatible (API $SDK_LEVEL)"
fi

if [[ "$ARCH" == *"arm64"* ]]; then
    echo "  ✅ ARM64 architecture detected"
elif [[ "$ARCH" == *"armeabi"* ]]; then
    echo "  ✅ ARM32 architecture detected"
else
    echo "  ❌ Unsupported architecture: $ARCH"
    COMPATIBLE=false
fi

echo ""
echo "Checking permissions..."

HAS_OVERLAY=$(adb shell dumpsys package com.axlebolt.standoff2 | grep -c "SYSTEM_ALERT_WINDOW")
if [ "$HAS_OVERLAY" -gt 0 ]; then
    echo "  ✅ Overlay permission supported"
else
    echo "  ⚠️  Overlay permission status unknown"
fi

echo ""
echo "Checking available space..."
AVAILABLE=$(adb shell df /data | tail -1 | awk '{print $4}')
echo "  Available space: $AVAILABLE"

if [ "$AVAILABLE" -lt 100000 ]; then
    echo "  ⚠️  Low storage space (recommend 100MB+)"
fi

echo ""
echo "Checking root status..."
ROOT=$(adb shell su -c "id" 2>/dev/null | grep -c "uid=0")
if [ "$ROOT" -gt 0 ]; then
    echo "  📱 Device is ROOTED (not required for this mod)"
else
    echo "  📱 Device is NOT ROOTED (perfect for rootless mod!)"
fi

echo ""
echo "Checking emulator..."
EMULATOR=$(adb shell getprop ro.kernel.qemu)
if [ "$EMULATOR" == "1" ]; then
    echo "  ⚠️  Emulator detected! Mod may not work properly"
    echo "     Recommendation: Use real device for best experience"
else
    echo "  ✅ Real device detected"
fi

echo ""
echo "=========================================="
if [ "$COMPATIBLE" != "false" ]; then
    echo "  Device is COMPATIBLE! ✅"
else
    echo "  Device is NOT COMPATIBLE! ❌"
fi
echo "=========================================="
