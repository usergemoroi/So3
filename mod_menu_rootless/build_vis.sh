#!/bin/bash

set -e

echo "=========================================="
echo "  Building classes.dex and libvis.so"
echo "=========================================="
echo ""

# Проверка зависимостей
if [ -z "$ANDROID_NDK_HOME" ]; then
    echo "ERROR: ANDROID_NDK_HOME not set!"
    echo "Please set ANDROID_NDK_HOME environment variable"
    exit 1
fi

if [ -z "$ANDROID_HOME" ]; then
    echo "ERROR: ANDROID_HOME not set!"
    echo "Please set ANDROID_HOME environment variable"
    exit 1
fi

echo "[1/4] Building native library (libvis.so)..."
cd jni
$ANDROID_NDK_HOME/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk \
    NDK_DEBUG=0 \
    APP_OPTIM=release \
    -j$(nproc)

if [ $? -ne 0 ]; then
    echo "ERROR: Native build failed!"
    exit 1
fi

cd ..
echo "✓ Native library built"
echo ""

echo "[2/4] Compiling Java classes..."
mkdir -p build/classes

find java -name "*.java" > build/sources.txt

javac -d build/classes \
    -source 1.8 -target 1.8 \
    -bootclasspath $ANDROID_HOME/platforms/android-30/android.jar \
    @build/sources.txt

if [ $? -ne 0 ]; then
    echo "ERROR: Java compilation failed!"
    exit 1
fi

echo "✓ Java classes compiled"
echo ""

echo "[3/4] Creating classes.dex..."

D8_TOOL="$ANDROID_HOME/build-tools/*/d8"
D8=$(ls $D8_TOOL 2>/dev/null | head -1)

if [ -z "$D8" ]; then
    echo "ERROR: d8 tool not found!"
    exit 1
fi

$D8 --lib $ANDROID_HOME/platforms/android-30/android.jar \
    --release \
    --output build/ \
    $(find build/classes -name "*.class")

mv build/classes.dex build/classes.dex

echo "✓ classes.dex created"
echo ""

echo "[4/4] Packaging output files..."
mkdir -p output

# Копируем native библиотеки с правильным именем
for arch in arm64-v8a armeabi-v7a; do
    if [ -f "libs/$arch/libvis.so" ]; then
        cp libs/$arch/libvis.so output/libvis.so.$arch
        echo "✓ Copied libvis.so for $arch"
    fi
done

# Копируем DEX
cp build/classes.dex output/
echo "✓ Copied classes.dex"

echo ""
echo "=========================================="
echo "  Build complete!"
echo "=========================================="
echo ""
echo "Output files:"
ls -lh output/
echo ""
echo "Native libraries:"
ls -lh libs/*/libvis.so 2>/dev/null || echo "  No libraries built"
echo ""
