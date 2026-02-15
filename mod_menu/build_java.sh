#!/bin/bash

echo "=========================================="
echo "  Building Java Classes"
echo "=========================================="
echo ""

# Check for Java
if ! command -v javac &> /dev/null; then
    echo "ERROR: javac not found!"
    echo "Please install JDK"
    exit 1
fi

# Check for Android SDK
if [ -z "$ANDROID_HOME" ]; then
    echo "ERROR: ANDROID_HOME not set!"
    echo "Please set ANDROID_HOME environment variable"
    exit 1
fi

echo "[*] Compiling Java source files..."

JAVA_SRC="java"
BUILD_DIR="build/classes"
ANDROID_JAR="$ANDROID_HOME/platforms/android-30/android.jar"

mkdir -p "$BUILD_DIR"

# Find all Java files
JAVA_FILES=$(find "$JAVA_SRC" -name "*.java")

if [ -z "$JAVA_FILES" ]; then
    echo "ERROR: No Java files found!"
    exit 1
fi

echo "  Found $(echo $JAVA_FILES | wc -w) Java files"

# Compile Java files
javac -source 1.8 -target 1.8 \
      -cp "$ANDROID_JAR" \
      -d "$BUILD_DIR" \
      $JAVA_FILES

if [ $? -eq 0 ]; then
    echo "[✓] Java compilation successful"
else
    echo "[✗] Java compilation failed!"
    exit 1
fi

echo ""
echo "[*] Converting to DEX..."

# Check for d8 (new) or dx (legacy)
D8="$ANDROID_HOME/build-tools/$(ls $ANDROID_HOME/build-tools | tail -1)/d8"
DX="$ANDROID_HOME/build-tools/$(ls $ANDROID_HOME/build-tools | tail -1)/dx"

DEX_OUTPUT="build/classes.dex"

if [ -f "$D8" ]; then
    echo "  Using d8 compiler"
    $D8 --release \
        --min-api 21 \
        --output build/ \
        $(find "$BUILD_DIR" -name "*.class")
    
    if [ $? -eq 0 ]; then
        echo "[✓] DEX creation successful"
    else
        echo "[✗] DEX creation failed!"
        exit 1
    fi
elif [ -f "$DX" ]; then
    echo "  Using dx compiler"
    $DX --dex \
        --output="$DEX_OUTPUT" \
        "$BUILD_DIR"
    
    if [ $? -eq 0 ]; then
        echo "[✓] DEX creation successful"
    else
        echo "[✗] DEX creation failed!"
        exit 1
    fi
else
    echo "ERROR: Neither d8 nor dx found!"
    echo "Please install Android build-tools"
    exit 1
fi

echo ""
echo "=========================================="
echo "  Build Complete!"
echo "=========================================="
echo ""
echo "Output files:"
echo "  - build/classes.dex (for injection)"
echo ""
echo "Size: $(ls -lh build/classes.dex | awk '{print $5}')"
echo ""
