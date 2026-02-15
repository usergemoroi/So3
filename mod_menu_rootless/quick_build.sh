#!/bin/bash

set -e

echo "========================================"
echo " Mod Menu Rootless - Quick Build Script"
echo "========================================"
echo ""

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build_output"
TOOLS_DIR="$PROJECT_DIR/tools_cache"

mkdir -p "$BUILD_DIR"
mkdir -p "$TOOLS_DIR"

# Detect Android SDK (if exists)
if [ -z "$ANDROID_HOME" ]; then
    if [ -d "$HOME/Android/Sdk" ]; then
        export ANDROID_HOME="$HOME/Android/Sdk"
    elif [ -d "/usr/lib/android-sdk" ]; then
        export ANDROID_HOME="/usr/lib/android-sdk"
    elif [ -d "$TOOLS_DIR/android-sdk" ]; then
        export ANDROID_HOME="$TOOLS_DIR/android-sdk"
    fi
fi

# Download and setup Android command line tools if needed
if [ ! -f "$TOOLS_DIR/cmdline-tools/bin/d8" ] && [ ! -f "$ANDROID_HOME/build-tools/"*"/d8" ]; then
    echo "[*] Downloading Android command line tools..."
    cd "$TOOLS_DIR"
    wget -q "https://dl.google.com/android/repository/commandlinetools-linux-9477386_latest.zip" -O cmdtools.zip
    unzip -q cmdtools.zip
    rm cmdtools.zip
    cd "$PROJECT_DIR"
fi

# Find d8 tool
D8_TOOL=""
if [ -n "$ANDROID_HOME" ] && [ -d "$ANDROID_HOME/build-tools" ]; then
    D8_TOOL=$(find "$ANDROID_HOME/build-tools" -name "d8" 2>/dev/null | head -1)
fi
if [ -z "$D8_TOOL" ] && [ -f "$TOOLS_DIR/cmdline-tools/bin/d8" ]; then
    D8_TOOL="$TOOLS_DIR/cmdline-tools/bin/d8"
fi

# Download and setup minimal NDK if needed
if [ -z "$ANDROID_NDK_HOME" ]; then
    if [ -d "$ANDROID_HOME/ndk" ]; then
        export ANDROID_NDK_HOME=$(ls -d "$ANDROID_HOME/ndk"/* | head -1)
    elif [ -d "$TOOLS_DIR/android-ndk-r21e" ]; then
        export ANDROID_NDK_HOME="$TOOLS_DIR/android-ndk-r21e"
    elif [ -d "/usr/lib/android-ndk" ]; then
        export ANDROID_NDK_HOME="/usr/lib/android-ndk"
    fi
fi

if [ -z "$ANDROID_NDK_HOME" ] || [ ! -f "$ANDROID_NDK_HOME/ndk-build" ]; then
    echo "[*] NDK not found. Downloading Android NDK r21e (this may take a while)..."
    cd "$TOOLS_DIR"
    wget -q --show-progress "https://dl.google.com/android/repository/android-ndk-r21e-linux-x86_64.zip" -O ndk.zip || {
        echo "[!] Failed to download NDK. Trying mirror..."
        wget -q --show-progress "https://github.com/android/ndk/releases/download/r21e/android-ndk-r21e-linux-x86_64.zip" -O ndk.zip
    }
    echo "[*] Extracting NDK..."
    unzip -q ndk.zip
    rm ndk.zip
    export ANDROID_NDK_HOME="$TOOLS_DIR/android-ndk-r21e"
    cd "$PROJECT_DIR"
fi

echo ""
echo "===========================================_"
echo "[1/4] Building native library with NDK..."
echo "============================================"
cd "$PROJECT_DIR/jni"
"$ANDROID_NDK_HOME/ndk-build" NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk V=1
cd "$PROJECT_DIR"

echo ""
echo "============================================"
echo "[2/4] Compiling Java classes..."
echo "============================================"
mkdir -p "$BUILD_DIR/classes"

# Get android.jar path
ANDROID_JAR=""
if [ -n "$ANDROID_HOME" ] && [ -d "$ANDROID_HOME/platforms" ]; then
    ANDROID_JAR=$(find "$ANDROID_HOME/platforms" -name "android.jar" | sort -V | tail -1)
fi
if [ -z "$ANDROID_JAR" ]; then
    echo "[*] android.jar not found, downloading Android SDK platform..."
    mkdir -p "$TOOLS_DIR/platform"
    wget -q "https://dl.google.com/android/repository/platform-28_r06.zip" -O "$TOOLS_DIR/platform.zip"
    unzip -q "$TOOLS_DIR/platform.zip" -d "$TOOLS_DIR/platform"
    rm "$TOOLS_DIR/platform.zip"
    ANDROID_JAR=$(find "$TOOLS_DIR/platform" -name "android.jar" | head -1)
fi

javac -d "$BUILD_DIR/classes" \
    -source 1.8 -target 1.8 \
    -bootclasspath "$ANDROID_JAR" \
    "$PROJECT_DIR/java/com/modmenu/loader"/*.java

echo ""
echo "============================================"
echo "[3/4] Creating DEX file..."
echo "============================================"

# Use d8 if available, otherwise try dx
if [ -n "$D8_TOOL" ] && [ -f "$D8_TOOL" ]; then
    "$D8_TOOL" \
        --lib "$ANDROID_JAR" \
        --release \
        --output "$BUILD_DIR" \
        "$BUILD_DIR/classes/com/modmenu/loader"/*.class
    mv "$BUILD_DIR/classes.dex" "$BUILD_DIR/modmenu.dex"
else
    # Fallback: try to find dx
    DX_TOOL=""
    if [ -n "$ANDROID_HOME" ]; then
        DX_TOOL=$(find "$ANDROID_HOME/build-tools" -name "dx" 2>/dev/null | head -1)
    fi
    
    if [ -n "$DX_TOOL" ]; then
        "$DX_TOOL" --dex \
            --output="$BUILD_DIR/modmenu.dex" \
            "$BUILD_DIR/classes"
    else
        echo "[!] ERROR: Neither d8 nor dx tool found!"
        echo "[!] Please install Android SDK build-tools"
        exit 1
    fi
fi

echo ""
echo "============================================"
echo "[4/4] Packaging output..."
echo "============================================"
mkdir -p "$BUILD_DIR/lib/arm64-v8a"
mkdir -p "$BUILD_DIR/lib/armeabi-v7a"

# Copy native libraries
if [ -f "$PROJECT_DIR/libs/arm64-v8a/libmodmenu_noroot.so" ]; then
    cp "$PROJECT_DIR/libs/arm64-v8a/libmodmenu_noroot.so" "$BUILD_DIR/lib/arm64-v8a/"
    echo "[✓] Copied ARM64 library"
fi

if [ -f "$PROJECT_DIR/libs/armeabi-v7a/libmodmenu_noroot.so" ]; then
    cp "$PROJECT_DIR/libs/armeabi-v7a/libmodmenu_noroot.so" "$BUILD_DIR/lib/armeabi-v7a/"
    echo "[✓] Copied ARM32 library"
fi

# Copy DEX
cp "$BUILD_DIR/modmenu.dex" "$BUILD_DIR/"
echo "[✓] DEX file ready"

echo ""
echo "============================================"
echo " ✓ Build complete!"
echo "============================================"
echo ""
echo "Output files in: $BUILD_DIR"
echo "  - modmenu.dex"
echo "  - lib/arm64-v8a/libmodmenu_noroot.so"
echo "  - lib/armeabi-v7a/libmodmenu_noroot.so"
echo ""
echo "Next steps:"
echo "  1. Copy modmenu.dex to /assets/ in your APK"
echo "  2. Copy lib/* to /lib/ in your APK"
echo "  3. Add necessary permissions to AndroidManifest.xml"
echo ""
