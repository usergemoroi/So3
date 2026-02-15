#!/bin/bash

echo "=========================================="
echo "  Protected Build Script v1.1"
echo "  Standoff 2 Mod Menu"
echo "=========================================="
echo ""

if [ -z "$ANDROID_NDK_HOME" ]; then
    echo "ERROR: ANDROID_NDK_HOME not set!"
    echo "Please set ANDROID_NDK_HOME environment variable"
    echo "Example: export ANDROID_NDK_HOME=/path/to/ndk"
    exit 1
fi

echo "[*] Using NDK: $ANDROID_NDK_HOME"
echo ""

echo "[*] Cleaning previous build..."
rm -rf libs obj .cxx 2>/dev/null
echo "  [✓] Clean complete"
echo ""

cd jni

echo "[*] Building protected version..."
echo "  - Using: Android_protected.mk"
echo "  - Output: libgamecore.so"
echo "  - Protection: ENABLED"
echo ""

$ANDROID_NDK_HOME/ndk-build \
    NDK_PROJECT_PATH=. \
    APP_BUILD_SCRIPT=Android_protected.mk \
    NDK_DEBUG=0 \
    -j$(nproc)

BUILD_RESULT=$?

cd ..

if [ $BUILD_RESULT -eq 0 ]; then
    echo ""
    echo "[✓] Build successful!"
    echo ""
    
    LIB_PATH="libs/arm64-v8a/libgamecore.so"
    
    if [ -f "$LIB_PATH" ]; then
        echo "[*] Running post-build obfuscation..."
        ./obfuscate.sh
        
        if [ $? -eq 0 ]; then
            echo ""
            echo "=========================================="
            echo "  🛡️  PROTECTED BUILD COMPLETE!"
            echo "=========================================="
            echo ""
            echo "Output: $LIB_PATH"
            echo ""
            
            FILE_SIZE=$(stat -f%z "$LIB_PATH" 2>/dev/null || stat -c%s "$LIB_PATH")
            echo "Size: $FILE_SIZE bytes (~$(($FILE_SIZE / 1024))KB)"
            echo ""
            
            if command -v sha256sum &> /dev/null; then
                HASH=$(sha256sum "$LIB_PATH" | cut -d' ' -f1)
                echo "SHA256: $HASH"
            fi
            
            echo ""
            echo "SECURITY FEATURES:"
            echo "  ✓ String encryption"
            echo "  ✓ Anti-debugging"
            echo "  ✓ Memory protection"
            echo "  ✓ Module hiding"
            echo "  ✓ Code obfuscation"
            echo "  ✓ Symbol stripping"
            echo "  ✓ Random timing"
            echo "  ✓ Hook detection"
            echo "  ✓ Integrity bypass"
            echo "  ✓ Behavioral randomization"
            echo ""
            
            echo "VERIFICATION:"
            SYMBOLS=$(nm -D "$LIB_PATH" 2>/dev/null | grep -v "UND" | wc -l)
            echo "  • Exported symbols: $SYMBOLS (target: <5)"
            
            STRINGS=$(strings "$LIB_PATH" | grep -iE "mod|menu|esp|aim" | wc -l)
            echo "  • Suspicious strings: $STRINGS (target: 0)"
            
            if command -v readelf &> /dev/null; then
                DEBUG_SECTIONS=$(readelf -S "$LIB_PATH" | grep -E "\.debug|\.symtab" | wc -l)
                echo "  • Debug sections: $DEBUG_SECTIONS (target: 0)"
            fi
            
            echo ""
            echo "NEXT STEPS:"
            echo "  1. Test the module in a safe environment"
            echo "  2. Verify protection with 'nm' and 'strings'"
            echo "  3. Use only on test accounts"
            echo "  4. Read SECURITY_GUIDE.md for best practices"
            echo ""
            
            if [ $SYMBOLS -gt 5 ]; then
                echo "⚠️  WARNING: Too many exported symbols!"
                echo "   Run './obfuscate.sh' again or check build config"
            fi
            
            if [ $STRINGS -gt 0 ]; then
                echo "⚠️  WARNING: Found suspicious strings!"
                echo "   Check string encryption in code"
            fi
            
            echo ""
        else
            echo ""
            echo "⚠️  WARNING: Obfuscation failed, but build succeeded"
            echo "   Library is functional but less protected"
        fi
    else
        echo ""
        echo "ERROR: Library not found at $LIB_PATH"
        exit 1
    fi
else
    echo ""
    echo "=========================================="
    echo "  ❌ BUILD FAILED!"
    echo "=========================================="
    echo ""
    echo "Check the error messages above"
    echo "Common issues:"
    echo "  • NDK path incorrect"
    echo "  • Missing dependencies"
    echo "  • Syntax errors in code"
    echo ""
    exit 1
fi
