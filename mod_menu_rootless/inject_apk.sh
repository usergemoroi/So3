#!/bin/bash
set -e

echo "=========================================="
echo "  APK Integration Script"
echo "=========================================="
echo ""

if [ -z "$1" ]; then
    echo "Usage: $0 <path_to_standoff2.apk>"
    exit 1
fi

APK_PATH="$1"
if [ ! -f "$APK_PATH" ]; then
    echo "ERROR: APK not found: $APK_PATH"
    exit 1
fi

WORK_DIR="apk_work"
OUTPUT_APK="standoff2_modded.apk"
SIGNED_APK="standoff2_modded_signed.apk"

# Clean previous work
rm -rf $WORK_DIR ${WORK_DIR}_decoded
mkdir -p $WORK_DIR

echo "[1/6] Extracting APK..."
unzip -q "$APK_PATH" -d $WORK_DIR

echo ""
echo "[2/6] Injecting native libraries..."
for arch in arm64-v8a armeabi-v7a; do
    if [ -f "output/lib/$arch/libv.so" ]; then
        mkdir -p $WORK_DIR/lib/$arch
        cp output/lib/$arch/libv.so $WORK_DIR/lib/$arch/
        echo "✓ Injected libv.so for $arch"
    fi
done

echo ""
echo "[3/6] Injecting DEX..."
if [ -f "output/a.dex" ]; then
    mkdir -p $WORK_DIR/assets
    cp output/a.dex $WORK_DIR/assets/a
    echo "✓ Injected a.dex as assets/a"
fi

echo ""
echo "[4/6] Patching AndroidManifest.xml..."

if command -v apktool &> /dev/null; then
    echo "Using apktool for manifest patching..."
    
    apktool d "$APK_PATH" -o ${WORK_DIR}_decoded -f > /dev/null 2>&1
    
    MANIFEST="${WORK_DIR}_decoded/AndroidManifest.xml"
    
    # Add permissions if not present
    if ! grep -q "SYSTEM_ALERT_WINDOW" "$MANIFEST"; then
        sed -i '/<manifest/a\\    <uses-permission android:name="android.permission.SYSTEM_ALERT_WINDOW"/>' "$MANIFEST"
        echo "✓ Added SYSTEM_ALERT_WINDOW permission"
    fi
    
    # Add service
    if ! grep -q "a.b.c.e" "$MANIFEST"; then
        sed -i '/<\/application>/i\\        <service android:name="a.b.c.e" android:enabled="true" android:exported="false"/>' "$MANIFEST"
        echo "✓ Added service a.b.c.e"
    fi
    
    # Find Application class and patch
    APP_CLASS=$(grep -oP 'android:name="\K[^"]+(?="' "$MANIFEST" | grep -v "^\\." | head -1)
    
    if [ -n "$APP_CLASS" ]; then
        echo "Found Application class: $APP_CLASS"
        
        # Convert to smali path
        SMALI_PATH=$(echo "$APP_CLASS" | tr '.' '/')".smali"
        
        # Find smali file
        for smali_dir in ${WORK_DIR}_decoded/smali*; do
            SMALI_FILE="$smali_dir/$SMALI_PATH"
            if [ -f "$SMALI_FILE" ]; then
                echo "Patching $SMALI_FILE..."
                
                # Add injection code after onCreate super call
                awk '
                /.method.*onCreate\(\)V/,/return-void/ {
                    if (/invoke-super.*onCreate/ && !done) {
                        print
                        print ""
                        print "    invoke-static {p0}, La/b/c/f;->inject(Landroid/content/Context;)V"
                        print ""
                        print "    invoke-static {p0}, La/b/c/d;->inject(Landroid/app/Application;)V"
                        done=1
                        next
                    }
                }
                {print}
                ' "$SMALI_FILE" > "${SMALI_FILE}.tmp"
                mv "${SMALI_FILE}.tmp" "$SMALI_FILE"
                echo "✓ Patched Application class"
                break
            fi
        done
    fi
    
    # Rebuild APK
    echo "Rebuilding APK with apktool..."
    apktool b ${WORK_DIR}_decoded -o $OUTPUT_APK > /dev/null 2>&1
else
    echo "apktool not found, using simple zip method..."
    
    cd $WORK_DIR
    zip -r -q ../$OUTPUT_APK .
    cd ..
    
    echo "WARNING: Manifest not patched - you need apktool for full integration"
fi

echo ""
echo "[5/6] Aligning APK..."
if command -v zipalign &> /dev/null; then
    ZIPALIGN=$(find $ANDROID_HOME/build-tools -name zipalign 2>/dev/null | head -1)
    if [ -n "$ZIPALIGN" ]; then
        $ZIPALIGN -f 4 $OUTPUT_APK ${OUTPUT_APK%.apk}_aligned.apk
        mv ${OUTPUT_APK%.apk}_aligned.apk $OUTPUT_APK
        echo "✓ APK aligned"
    fi
fi

echo ""
echo "[6/6] Signing APK..."
KEYSTORE="debug.keystore"
if [ ! -f "$KEYSTORE" ]; then
    keytool -genkey -v \
        -keystore $KEYSTORE \
        -alias androiddebugkey \
        -keyalg RSA \
        -keysize 2048 \
        -validity 10000 \
        -storepass android \
        -keypass android \
        -dname "CN=Android Debug,O=Android,C=US" \
        > /dev/null 2>&1
    echo "✓ Created debug keystore"
fi

if command -v apksigner &> /dev/null; then
    APKSIGNER=$(find $ANDROID_HOME/build-tools -name apksigner 2>/dev/null | head -1)
    if [ -n "$APKSIGNER" ]; then
        $APKSIGNER sign \
            --ks $KEYSTORE \
            --ks-pass pass:android \
            --key-pass pass:android \
            --out $SIGNED_APK \
            $OUTPUT_APK
        echo "✓ APK signed with apksigner"
    fi
else
    jarsigner -sigalg SHA1withRSA \
        -digestalg SHA1 \
        -keystore $KEYSTORE \
        -storepass android \
        -keypass android \
        $OUTPUT_APK \
        androiddebugkey > /dev/null 2>&1
    mv $OUTPUT_APK $SIGNED_APK
    echo "✓ APK signed with jarsigner"
fi

# Cleanup
rm -rf $WORK_DIR ${WORK_DIR}_decoded $OUTPUT_APK

echo ""
echo "=========================================="
echo "  Integration Complete!"
echo "=========================================="
echo ""
echo "Output: $SIGNED_APK"
echo ""
echo "Install command:"
echo "  adb install -r $SIGNED_APK"
echo ""
echo "Don't forget to grant overlay permission in device settings!"
echo ""
