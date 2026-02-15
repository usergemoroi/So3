#!/bin/bash

set -e

echo "=============================================="
echo "  AUTO BUILD & INJECT - Rootless Mod Menu"
echo "=============================================="
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Проверка аргументов
if [ -z "$1" ]; then
    echo -e "${RED}Usage: $0 <path_to_standoff2.apk>${NC}"
    exit 1
fi

APK_PATH="$1"

if [ ! -f "$APK_PATH" ]; then
    echo -e "${RED}ERROR: APK not found: $APK_PATH${NC}"
    exit 1
fi

# Проверка зависимостей
echo -e "${BLUE}[1/7] Checking dependencies...${NC}"

check_command() {
    if ! command -v $1 &> /dev/null; then
        echo -e "${RED}ERROR: $1 not found!${NC}"
        echo "Please install $1"
        exit 1
    fi
}

if [ -z "$ANDROID_NDK_HOME" ]; then
    echo -e "${RED}ERROR: ANDROID_NDK_HOME not set!${NC}"
    exit 1
fi

if [ -z "$ANDROID_HOME" ]; then
    echo -e "${RED}ERROR: ANDROID_HOME not set!${NC}"
    exit 1
fi

check_command javac
check_command python3
check_command zip
check_command unzip

echo -e "${GREEN}✓ All dependencies OK${NC}"
echo ""

# Сборка нативной библиотеки
echo -e "${BLUE}[2/7] Building native library...${NC}"
cd jni

# Обновляем main_noroot.cpp для использования улучшенной защиты
sed -i 's/protection_noroot.hpp/protection_advanced.hpp/g' main_noroot.cpp || true

$ANDROID_NDK_HOME/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk \
    NDK_DEBUG=0 \
    APP_OPTIM=release \
    -j$(nproc)

if [ $? -ne 0 ]; then
    echo -e "${RED}✗ Native build failed!${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Native library built${NC}"
cd ..

# Компиляция Java классов
echo ""
echo -e "${BLUE}[3/7] Compiling Java classes...${NC}"
mkdir -p build/classes

find java -name "*.java" > build/sources.txt

javac -d build/classes \
    -source 1.8 -target 1.8 \
    -bootclasspath $ANDROID_HOME/platforms/android-30/android.jar \
    @build/sources.txt

if [ $? -ne 0 ]; then
    echo -e "${RED}✗ Java compilation failed!${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Java classes compiled${NC}"

# Создание DEX файла
echo ""
echo -e "${BLUE}[4/7] Creating DEX file...${NC}"

D8_TOOL="$ANDROID_HOME/build-tools/*/d8"
D8=$(ls $D8_TOOL 2>/dev/null | head -1)

if [ -z "$D8" ]; then
    echo -e "${RED}ERROR: d8 tool not found!${NC}"
    exit 1
fi

$D8 --lib $ANDROID_HOME/platforms/android-30/android.jar \
    --release \
    --output build/ \
    $(find build/classes -name "*.class")

mv build/classes.dex build/modmenu.dex

echo -e "${GREEN}✓ DEX file created${NC}"

# Извлечение APK
echo ""
echo -e "${BLUE}[5/7] Extracting APK...${NC}"

WORK_DIR="apk_work"
rm -rf $WORK_DIR
mkdir -p $WORK_DIR

unzip -q "$APK_PATH" -d $WORK_DIR

echo -e "${GREEN}✓ APK extracted${NC}"

# Инъекция файлов
echo ""
echo -e "${BLUE}[6/7] Injecting mod files...${NC}"

# Копируем DEX
mkdir -p $WORK_DIR/assets
cp build/modmenu.dex $WORK_DIR/assets/

# Копируем библиотеки
for arch in arm64-v8a armeabi-v7a; do
    if [ -f "libs/$arch/libmodmenu_noroot.so" ]; then
        mkdir -p $WORK_DIR/lib/$arch
        cp libs/$arch/libmodmenu_noroot.so $WORK_DIR/lib/$arch/
        echo -e "${GREEN}✓ Copied library for $arch${NC}"
    fi
done

# Модификация AndroidManifest.xml
if [ -f "$WORK_DIR/AndroidManifest.xml" ]; then
    echo -e "${YELLOW}  → Adding permissions to manifest...${NC}"
    
    # Используем apktool для декомпиляции/компиляции
    if command -v apktool &> /dev/null; then
        DECODED_DIR="${WORK_DIR}_decoded"
        rm -rf $DECODED_DIR
        
        apktool d "$APK_PATH" -o $DECODED_DIR -f > /dev/null 2>&1
        
        if [ -f "$DECODED_DIR/AndroidManifest.xml" ]; then
            # Добавляем разрешения если их нет
            if ! grep -q "SYSTEM_ALERT_WINDOW" "$DECODED_DIR/AndroidManifest.xml"; then
                sed -i '/<manifest/a\    <uses-permission android:name="android.permission.SYSTEM_ALERT_WINDOW"/>' "$DECODED_DIR/AndroidManifest.xml"
            fi
            if ! grep -q "INTERNET" "$DECODED_DIR/AndroidManifest.xml"; then
                sed -i '/<manifest/a\    <uses-permission android:name="android.permission.INTERNET"/>' "$DECODED_DIR/AndroidManifest.xml"
            fi
            
            # Добавляем сервис
            if ! grep -q "OverlayService" "$DECODED_DIR/AndroidManifest.xml"; then
                sed -i '/<\/application>/i\        <service android:name="com.modmenu.loader.OverlayService" android:enabled="true" android:exported="false"/>' "$DECODED_DIR/AndroidManifest.xml"
            fi
            
            # Находим Application класс
            APP_CLASS=$(grep -oP 'android:name="\K[^"]+(?=".*android:label)' "$DECODED_DIR/AndroidManifest.xml" | head -1)
            
            if [ -n "$APP_CLASS" ]; then
                echo -e "${YELLOW}  → Found Application class: $APP_CLASS${NC}"
                
                # Конвертируем в путь к smali файлу
                SMALI_PATH=$(echo "$APP_CLASS" | tr '.' '/')
                SMALI_FILE="$DECODED_DIR/smali/${SMALI_PATH}.smali"
                
                if [ ! -f "$SMALI_FILE" ]; then
                    SMALI_FILE="$DECODED_DIR/smali_classes2/${SMALI_PATH}.smali"
                fi
                
                if [ -f "$SMALI_FILE" ]; then
                    echo -e "${YELLOW}  → Patching Application class...${NC}"
                    
                    # Ищем метод onCreate
                    if grep -q "\.method.*onCreate()V" "$SMALI_FILE"; then
                        # Добавляем вызов после invoke-super
                        awk '/\.method.*onCreate\(\)V/,/return-void/ {
                            if (/invoke-super.*onCreate/) {
                                print
                                print ""
                                print "    invoke-static {p0}, Lcom/modmenu/loader/DexInjector;->inject(Landroid/content/Context;)V"
                                print ""
                                print "    invoke-static {p0}, Lcom/modmenu/loader/ModMenuLoader;->inject(Landroid/app/Application;)V"
                                next
                            }
                        }
                        {print}' "$SMALI_FILE" > "${SMALI_FILE}.tmp"
                        mv "${SMALI_FILE}.tmp" "$SMALI_FILE"
                        
                        echo -e "${GREEN}  ✓ Application class patched${NC}"
                    fi
                fi
            fi
            
            # Пересобираем APK с помощью apktool
            echo -e "${YELLOW}  → Rebuilding APK...${NC}"
            OUTPUT_APK="standoff2_modded.apk"
            apktool b $DECODED_DIR -o $OUTPUT_APK > /dev/null 2>&1
            
            if [ ! -f "$OUTPUT_APK" ]; then
                echo -e "${RED}✗ APK rebuild failed${NC}"
                exit 1
            fi
        fi
    else
        echo -e "${YELLOW}  ! apktool not found, skipping manifest patching${NC}"
        
        # Просто пересобираем zip
        cd $WORK_DIR
        OUTPUT_APK="../standoff2_modded.apk"
        zip -r -q $OUTPUT_APK *
        cd ..
    fi
else
    # Пересобираем zip без apktool
    cd $WORK_DIR
    OUTPUT_APK="../standoff2_modded.apk"
    zip -r -q $OUTPUT_APK *
    cd ..
fi

echo -e "${GREEN}✓ Mod files injected${NC}"

# Подпись APK
echo ""
echo -e "${BLUE}[7/7] Signing APK...${NC}"

KEYSTORE="debug.keystore"
SIGNED_APK="standoff2_modded_signed.apk"

# Создаем keystore если нет
if [ ! -f "$KEYSTORE" ]; then
    echo -e "${YELLOW}  → Creating debug keystore...${NC}"
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
fi

# Выравниваем APK
if command -v zipalign &> /dev/null; then
    echo -e "${YELLOW}  → Aligning APK...${NC}"
    ZIPALIGN=$(find $ANDROID_HOME/build-tools -name zipalign | head -1)
    $ZIPALIGN -f 4 standoff2_modded.apk standoff2_modded_aligned.apk > /dev/null 2>&1
    mv standoff2_modded_aligned.apk standoff2_modded.apk
fi

# Подписываем APK
if command -v apksigner &> /dev/null; then
    echo -e "${YELLOW}  → Signing with apksigner...${NC}"
    APKSIGNER=$(find $ANDROID_HOME/build-tools -name apksigner | head -1)
    $APKSIGNER sign \
        --ks $KEYSTORE \
        --ks-pass pass:android \
        --key-pass pass:android \
        --out $SIGNED_APK \
        standoff2_modded.apk > /dev/null 2>&1
else
    echo -e "${YELLOW}  → Signing with jarsigner...${NC}"
    jarsigner -sigalg SHA1withRSA \
        -digestalg SHA1 \
        -keystore $KEYSTORE \
        -storepass android \
        -keypass android \
        standoff2_modded.apk \
        androiddebugkey > /dev/null 2>&1
    mv standoff2_modded.apk $SIGNED_APK
fi

if [ ! -f "$SIGNED_APK" ]; then
    echo -e "${RED}✗ Signing failed${NC}"
    exit 1
fi

echo -e "${GREEN}✓ APK signed${NC}"

# Очистка
echo ""
echo -e "${BLUE}Cleaning up...${NC}"
rm -rf $WORK_DIR ${WORK_DIR}_decoded build/classes build/sources.txt standoff2_modded.apk

echo ""
echo "=============================================="
echo -e "${GREEN}  BUILD & INJECT COMPLETE!${NC}"
echo "=============================================="
echo ""
echo -e "${GREEN}Output:${NC} $SIGNED_APK"
echo ""
echo -e "${YELLOW}Next steps:${NC}"
echo "  1. Install: adb install -r $SIGNED_APK"
echo "  2. Grant overlay permission in device settings"
echo "  3. Open Standoff 2 and enjoy!"
echo ""
echo -e "${BLUE}Libraries included:${NC}"
ls -lh libs/*/libmodmenu_noroot.so 2>/dev/null | awk '{print "  •", $9, "-", $5}'
echo ""
echo -e "${BLUE}Files added to APK:${NC}"
echo "  • assets/modmenu.dex"
echo "  • lib/arm64-v8a/libmodmenu_noroot.so"
echo "  • lib/armeabi-v7a/libmodmenu_noroot.so (if built)"
echo ""
