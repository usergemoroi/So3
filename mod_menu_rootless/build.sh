#!/bin/bash
set -e

echo "=========================================="
echo "  Secure Mod Menu Build System"
echo "=========================================="
echo ""

# Check environment
if [ -z "$ANDROID_NDK_HOME" ]; then
    echo "ERROR: ANDROID_NDK_HOME not set!"
    echo "Example: export ANDROID_NDK_HOME=/opt/android-ndk"
    exit 1
fi

if [ -z "$ANDROID_HOME" ]; then
    echo "ERROR: ANDROID_HOME not set!"
    echo "Example: export ANDROID_HOME=/opt/android-sdk"
    exit 1
fi

BUILD_DIR="build"
OUTPUT_DIR="output"
rm -rf $BUILD_DIR $OUTPUT_DIR
mkdir -p $BUILD_DIR/classes $OUTPUT_DIR/lib

echo "[1/4] Building native library (libv.so)..."
cd jni
$ANDROID_NDK_HOME/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk clean 2>/dev/null || true
$ANDROID_NDK_HOME/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk -j$(nproc)
cd ..

echo ""
echo "[2/4] Compiling obfuscated Java classes..."
find java/a -name "*.java" > $BUILD_DIR/sources.txt

javac -d $BUILD_DIR/classes \
    -source 1.8 -target 1.8 \
    -bootclasspath $ANDROID_HOME/platforms/android-30/android.jar \
    @$BUILD_DIR/sources.txt 2>&1 | head -20 || echo "Java compilation may need SDK setup"

echo ""
echo "[3/4] Creating DEX file (a.dex)..."

# Find d8 tool
D8_TOOL=$(find $ANDROID_HOME/build-tools -name "d8" 2>/dev/null | head -1)
if [ -z "$D8_TOOL" ]; then
    D8_TOOL=$(find $ANDROID_HOME/build-tools -name "dx" 2>/dev/null | head -1)
fi

if [ -n "$D8_TOOL" ]; then
    $D8_TOOL --lib $ANDROID_HOME/platforms/android-30/android.jar \
        --release \
        --output $BUILD_DIR/ \
        $(find $BUILD_DIR/classes -name "*.class" 2>/dev/null) 2>/dev/null || echo "DEX creation skipped (may need proper SDK)"
    if [ -f "$BUILD_DIR/classes.dex" ]; then
        mv $BUILD_DIR/classes.dex $BUILD_DIR/a.dex
    fi
else
    echo "WARNING: d8/dx not found, skipping DEX creation"
fi

echo ""
echo "[4/4] Packaging..."

# Copy native libraries
for arch in arm64-v8a armeabi-v7a; do
    if [ -f "libs/$arch/libv.so" ]; then
        mkdir -p $OUTPUT_DIR/lib/$arch
        cp libs/$arch/libv.so $OUTPUT_DIR/lib/$arch/
        echo "✓ Copied libv.so for $arch"
    fi
done

# Copy DEX
if [ -f "$BUILD_DIR/a.dex" ]; then
    cp $BUILD_DIR/a.dex $OUTPUT_DIR/
    echo "✓ Copied a.dex"
fi

echo ""
echo "=========================================="
echo "  Build Complete!"
echo "=========================================="
echo ""
echo "Output files:"
ls -lh $OUTPUT_DIR/ 2>/dev/null || echo "(check output/ directory)"
echo ""
echo "Next step: Run ./inject_apk.sh <path_to_standoff2.apk>"
echo ""
