#!/bin/bash

echo "=========================================="
echo "  APK Injection Script"
echo "=========================================="
echo ""

if [ -z "$1" ]; then
    echo "Usage: $0 <path_to_apk>"
    echo "Example: $0 standoff2.apk"
    exit 1
fi

APK_FILE="$1"

if [ ! -f "$APK_FILE" ]; then
    echo "ERROR: APK file not found: $APK_FILE"
    exit 1
fi

# Check for required tools
if ! command -v apktool &> /dev/null; then
    echo "ERROR: apktool not found!"
    echo "Install: apt install apktool"
    exit 1
fi

if ! command -v zipalign &> /dev/null; then
    echo "WARNING: zipalign not found (optional)"
fi

if ! command -v apksigner &> /dev/null; then
    echo "WARNING: apksigner not found"
    echo "The APK will need to be signed manually"
fi

APK_NAME=$(basename "$APK_FILE" .apk)
WORK_DIR="work_${APK_NAME}"
OUTPUT_APK="${APK_NAME}_modded.apk"

echo "[*] Decompiling APK..."
rm -rf "$WORK_DIR"
apktool d "$APK_FILE" -o "$WORK_DIR" -f

if [ $? -ne 0 ]; then
    echo "ERROR: Failed to decompile APK"
    exit 1
fi

echo "[✓] APK decompiled"
echo ""

echo "[*] Injecting native library..."
LIB_PATH="libs/arm64-v8a/libgamecore.so"

if [ ! -f "$LIB_PATH" ]; then
    echo "ERROR: Native library not found: $LIB_PATH"
    echo "Please build the library first: ./build_noroot.sh"
    exit 1
fi

mkdir -p "$WORK_DIR/lib/arm64-v8a"
cp "$LIB_PATH" "$WORK_DIR/lib/arm64-v8a/"

echo "[✓] Native library injected"
echo ""

echo "[*] Injecting DEX classes..."
DEX_PATH="build/classes.dex"

if [ ! -f "$DEX_PATH" ]; then
    echo "ERROR: DEX file not found: $DEX_PATH"
    echo "Please build Java classes first: ./build_java.sh"
    exit 1
fi

# Find highest existing dex number
HIGHEST_DEX=$(ls "$WORK_DIR" | grep -E "classes[0-9]*\.dex" | sed 's/classes//;s/\.dex//' | sort -n | tail -1)

if [ -z "$HIGHEST_DEX" ]; then
    NEW_DEX="classes2.dex"
elif [ "$HIGHEST_DEX" = "" ]; then
    NEW_DEX="classes2.dex"
else
    NEW_DEX="classes$((HIGHEST_DEX + 1)).dex"
fi

echo "  Injecting as: $NEW_DEX"
cp "$DEX_PATH" "$WORK_DIR/$NEW_DEX"

echo "[✓] DEX injected"
echo ""

echo "[*] Modifying AndroidManifest.xml..."
MANIFEST="$WORK_DIR/AndroidManifest.xml"

# Backup original manifest
cp "$MANIFEST" "$MANIFEST.bak"

# Check if we need to add permissions
if ! grep -q "android.permission.SYSTEM_ALERT_WINDOW" "$MANIFEST"; then
    echo "  Adding SYSTEM_ALERT_WINDOW permission..."
    sed -i '/<manifest/a\    <uses-permission android:name="android.permission.SYSTEM_ALERT_WINDOW"/>' "$MANIFEST"
fi

# Find the original application tag
ORIG_APP=$(grep -o 'android:name="[^"]*"' "$MANIFEST" | grep application | head -1 | cut -d'"' -f2)

if [ -z "$ORIG_APP" ]; then
    echo "  No custom Application class found, adding our hook..."
    sed -i 's/<application/<application android:name="com.modmenu.hook.ApplicationHook"/' "$MANIFEST"
else
    echo "  Original Application class: $ORIG_APP"
    echo "  WARNING: Manual patching may be required!"
    echo "  Consider creating a wrapper class for $ORIG_APP"
fi

# Add overlay service
if ! grep -q "com.modmenu.overlay.OverlayService" "$MANIFEST"; then
    echo "  Adding OverlayService..."
    sed -i '/<\/application>/i\        <service android:name="com.modmenu.overlay.OverlayService" android:enabled="true" android:exported="false"/>' "$MANIFEST"
fi

echo "[✓] Manifest modified"
echo ""

echo "[*] Rebuilding APK..."
apktool b "$WORK_DIR" -o "$OUTPUT_APK"

if [ $? -ne 0 ]; then
    echo "ERROR: Failed to rebuild APK"
    exit 1
fi

echo "[✓] APK rebuilt"
echo ""

# Align APK
if command -v zipalign &> /dev/null; then
    echo "[*] Aligning APK..."
    ALIGNED_APK="${OUTPUT_APK}.aligned"
    zipalign -v 4 "$OUTPUT_APK" "$ALIGNED_APK"
    mv "$ALIGNED_APK" "$OUTPUT_APK"
    echo "[✓] APK aligned"
    echo ""
fi

# Sign APK
echo "[*] Signing APK..."

# Check for custom keystore
if [ -f "modmenu.keystore" ]; then
    echo "  Using existing keystore: modmenu.keystore"
    KEYSTORE="modmenu.keystore"
    KEYSTORE_PASS="modmenu123"
    KEY_ALIAS="modmenu"
else
    echo "  Generating new keystore..."
    KEYSTORE="modmenu.keystore"
    KEYSTORE_PASS="modmenu123"
    KEY_ALIAS="modmenu"
    
    keytool -genkey -v \
        -keystore "$KEYSTORE" \
        -alias "$KEY_ALIAS" \
        -keyalg RSA \
        -keysize 2048 \
        -validity 10000 \
        -storepass "$KEYSTORE_PASS" \
        -keypass "$KEYSTORE_PASS" \
        -dname "CN=ModMenu, OU=Mod, O=Mod, L=City, S=State, C=US" \
        2>/dev/null
fi

if command -v apksigner &> /dev/null; then
    apksigner sign \
        --ks "$KEYSTORE" \
        --ks-pass pass:"$KEYSTORE_PASS" \
        --ks-key-alias "$KEY_ALIAS" \
        "$OUTPUT_APK"
    
    if [ $? -eq 0 ]; then
        echo "[✓] APK signed with apksigner"
    else
        echo "[✗] Signing failed!"
    fi
else
    jarsigner -verbose \
        -sigalg SHA1withRSA \
        -digestalg SHA1 \
        -keystore "$KEYSTORE" \
        -storepass "$KEYSTORE_PASS" \
        "$OUTPUT_APK" \
        "$KEY_ALIAS" \
        2>/dev/null
    
    if [ $? -eq 0 ]; then
        echo "[✓] APK signed with jarsigner"
    else
        echo "[✗] Signing failed!"
    fi
fi

echo ""
echo "=========================================="
echo "  Injection Complete!"
echo "=========================================="
echo ""
echo "Output APK: $OUTPUT_APK"
echo "Size: $(ls -lh $OUTPUT_APK | awk '{print $5}')"
echo ""
echo "INSTALLATION:"
echo "  1. Uninstall the original app"
echo "  2. Install: adb install $OUTPUT_APK"
echo "  3. Grant overlay permission when prompted"
echo "  4. Launch the game"
echo "  5. Look for the menu button (☰)"
echo ""
echo "NOTES:"
echo "  - The APK is signed with a debug key"
echo "  - You may need to allow 'Unknown Sources'"
echo "  - First launch may take longer"
echo ""
