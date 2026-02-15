#!/bin/bash

echo "=========================================="
echo "  No-Root Mod Menu - Full Build"
echo "=========================================="
echo ""

# Check for NDK
if [ -z "$ANDROID_NDK_HOME" ]; then
    echo "ERROR: ANDROID_NDK_HOME not set!"
    echo "Please set ANDROID_NDK_HOME environment variable"
    exit 1
fi

# Check for SDK
if [ -z "$ANDROID_HOME" ]; then
    echo "ERROR: ANDROID_HOME not set!"
    echo "Please set ANDROID_HOME environment variable"
    exit 1
fi

echo "[*] Using NDK: $ANDROID_NDK_HOME"
echo "[*] Using SDK: $ANDROID_HOME"
echo ""

# Clean previous builds
echo "[*] Cleaning previous builds..."
rm -rf libs obj .cxx build 2>/dev/null
echo "[✓] Clean complete"
echo ""

# Build native library
echo "=========================================="
echo "  Step 1: Building Native Library"
echo "=========================================="
echo ""

cd jni

$ANDROID_NDK_HOME/ndk-build \
    NDK_PROJECT_PATH=. \
    APP_BUILD_SCRIPT=Android_noroot.mk \
    NDK_DEBUG=0 \
    -j$(nproc)

BUILD_RESULT=$?
cd ..

if [ $BUILD_RESULT -ne 0 ]; then
    echo ""
    echo "[✗] Native library build failed!"
    exit 1
fi

echo ""
echo "[✓] Native library built: libs/arm64-v8a/libgamecore.so"
echo ""

# Build Java classes
echo "=========================================="
echo "  Step 2: Building Java Classes"
echo "=========================================="
echo ""

./build_java.sh

if [ $? -ne 0 ]; then
    echo ""
    echo "[✗] Java build failed!"
    exit 1
fi

echo ""
echo "[✓] Java classes built: build/classes.dex"
echo ""

# Apply obfuscation
echo "=========================================="
echo "  Step 3: Applying Protection"
echo "=========================================="
echo ""

LIB_PATH="libs/arm64-v8a/libgamecore.so"

if [ -f "$LIB_PATH" ]; then
    echo "[*] Stripping symbols..."
    $ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip \
        --strip-all \
        "$LIB_PATH" 2>/dev/null
    
    echo "[✓] Symbols stripped"
    
    SYMBOLS=$(nm -D "$LIB_PATH" 2>/dev/null | grep -v "UND" | wc -l)
    echo "  Exported symbols: $SYMBOLS"
    
    FILE_SIZE=$(stat -c%s "$LIB_PATH" 2>/dev/null || stat -f%z "$LIB_PATH")
    echo "  Library size: $(($FILE_SIZE / 1024))KB"
fi

echo ""
echo "=========================================="
echo "  Build Complete!"
echo "=========================================="
echo ""
echo "Built components:"
echo "  ✓ Native library: libs/arm64-v8a/libgamecore.so"
echo "  ✓ Java DEX: build/classes.dex"
echo ""
echo "NEXT STEPS:"
echo "  1. Inject into APK:"
echo "     ./inject_apk.sh <path_to_standoff2.apk>"
echo ""
echo "  2. Install modded APK:"
echo "     adb install standoff2_modded.apk"
echo ""
echo "  3. Grant overlay permission when prompted"
echo ""
echo "  4. Launch game and tap menu button (☰)"
echo ""
echo "FEATURES:"
echo "  ✓ No root required"
echo "  ✓ Overlay menu"
echo "  ✓ ESP functions"
echo "  ✓ Aimbot functions"
echo "  ✓ Full protection system"
echo "  ✓ Auto-launch with game"
echo ""
