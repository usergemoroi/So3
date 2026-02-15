#!/bin/bash

set -e

echo "=========================================="
echo "  Building classes.dex (Java only)"
echo "=========================================="
echo ""

# Поиск Android SDK в стандартных местах
POSSIBLE_SDK_PATHS=(
    "$HOME/Android/Sdk"
    "$HOME/android-sdk"
    "/opt/android-sdk"
    "/usr/local/android-sdk"
    "$ANDROID_HOME"
)

ANDROID_SDK=""
for path in "${POSSIBLE_SDK_PATHS[@]}"; do
    if [ -d "$path" ]; then
        ANDROID_SDK="$path"
        echo "Found Android SDK at: $ANDROID_SDK"
        break
    fi
done

if [ -z "$ANDROID_SDK" ]; then
    echo "ERROR: Android SDK not found!"
    echo ""
    echo "Please install Android SDK or set ANDROID_HOME variable"
    echo "See SETUP_GUIDE.md for instructions"
    exit 1
fi

# Поиск android.jar
ANDROID_JAR=""
for api in 30 31 32 33 34; do
    if [ -f "$ANDROID_SDK/platforms/android-$api/android.jar" ]; then
        ANDROID_JAR="$ANDROID_SDK/platforms/android-$api/android.jar"
        echo "Found android.jar (API $api)"
        break
    fi
done

if [ -z "$ANDROID_JAR" ]; then
    echo "ERROR: android.jar not found in SDK!"
    echo "Install Android platform: sdkmanager \"platforms;android-30\""
    exit 1
fi

# Поиск d8 инструмента
D8_TOOL=""
for version in $ANDROID_SDK/build-tools/*/; do
    if [ -f "$version/d8" ]; then
        D8_TOOL="$version/d8"
        echo "Found d8 at: $D8_TOOL"
        break
    fi
done

if [ -z "$D8_TOOL" ]; then
    echo "ERROR: d8 tool not found!"
    echo "Install build-tools: sdkmanager \"build-tools;30.0.3\""
    exit 1
fi

echo ""
echo "[1/2] Compiling Java classes..."
mkdir -p build/classes

find java -name "*.java" > build/sources.txt

javac -d build/classes \
    -source 1.8 -target 1.8 \
    -bootclasspath "$ANDROID_JAR" \
    @build/sources.txt

if [ $? -ne 0 ]; then
    echo "ERROR: Java compilation failed!"
    exit 1
fi

echo "✓ Java classes compiled"
echo ""

echo "[2/2] Creating classes.dex..."

$D8_TOOL --lib "$ANDROID_JAR" \
    --release \
    --output build/ \
    $(find build/classes -name "*.class")

if [ $? -ne 0 ]; then
    echo "ERROR: DEX creation failed!"
    exit 1
fi

echo "✓ classes.dex created"
echo ""

echo "=========================================="
echo "  Build complete!"
echo "=========================================="
echo ""
echo "Output: build/classes.dex"
ls -lh build/classes.dex
echo ""

# Предложение скопировать в APK
echo "Copy to APK assets:"
echo "  cp build/classes.dex /home/engine/project/assets/"
echo ""
