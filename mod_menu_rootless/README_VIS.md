# Инструкция по сборке и инъекции mod menu (libvis.so + classes.dex)

## Обзор

Этот проект содержит исходники для создания мод меню для Standoff 2 без root доступа.

### Структура

- `java/` - Java исходники для загрузчика мода
- `jni/` - C++ исходники для native библиотеки (ESP, Aimbot, защита)
- `build_vis.sh` - Скрипт сборки
- `inject_to_apk.sh` - Скрипт инъекции в распакованный APK
- `build_and_inject_all.sh` - Полный автоматизированный процесс

## Требования

- **Android NDK** (установлен и настроен через `ANDROID_NDK_HOME`)
- **Android SDK** (установлен и настроен через `ANDROID_HOME`)
- **Java JDK 8+** (для javac)
- **apktool** (для пересборки APK)
- **apksigner** или **jarsigner** (для подписи APK)

## Установка переменных окружения

```bash
export ANDROID_NDK_HOME=/path/to/ndk
export ANDROID_HOME=/path/to/sdk
```

## Быстрый старт

### Шаг 1: Сборка + Инъекция (автоматически)

```bash
cd /home/engine/project/mod_menu_rootless
./build_and_inject_all.sh
```

Это сделает всё:
1. Скомпилирует Java исходники в `classes.dex`
2. Скомпилирует C++ код в `libvis.so` (ARM64 и ARM32)
3. Скопирует файлы в распакованный APK в `/home/engine/project/`

### Шаг 2: Проверка

После завершения проверьте файлы в `/home/engine/project`:

```bash
ls -lh /home/engine/project/assets/classes.dex
ls -lh /home/engine/project/lib/arm64-v8a/libvis.so
cat /home/engine/project/INJECTION_INFO.txt
```

### Шаг 3: Пересборка APK

Так как APK уже распакован в `/home/engine/project/`, вам нужно:

#### Вариант A: Использовать apktool (рекомендуется)

```bash
cd /home/engine/project

# 1. Проверьте и исправьте AndroidManifest.xml если нужно
#    Добавьте разрешения если их нет:
#    <uses-permission android:name="android.permission.SYSTEM_ALERT_WINDOW"/>
#    <uses-permission android:name="android.permission.INTERNET"/>

# 2. Найдите главный класс Application в smali/
#    В его onCreate() добавьте после invoke-super:
#    invoke-static {p0}, Lcom/modmenu/loader/DexInjector;->inject(Landroid/content/Context;)V
#    invoke-static {p0}, Lcom/modmenu/loader/ModMenuLoader;->inject(Landroid/app/Application;)V

# 3. Пересоберите APK
apktool b . -o standoff2_modded.apk

# 4. Подпишите APK
apksigner sign --ks debug.keystore --ks-pass pass:android --key-pass pass:android standoff2_modded.apk

# Или если apksigner недоступен:
# jarsigner -sigalg SHA1withRSA -digestalg SHA1 -keystore debug.keystore -storepass android standoff2_modded.apk androiddebugkey
```

#### Вариант B: Просто упаковать как ZIP

```bash
cd /home/engine/project
zip -r standoff2_modded.apk *

# Затем подпишите
apksigner sign --ks debug.keystore --ks-pass pass:android --key-pass pass:android standoff2_modded.apk
```

### Шаг 4: Установка

```bash
adb install -r standoff2_modded.apk
```

После установки:
1. Откройте настройки приложения
2. Разрешите наложение поверх других окон (SYSTEM_ALERT_WINDOW)
3. Запустите Standoff 2
4. Появится кнопка мод меню

## Ручная сборка (по шагам)

Если вы хотите сделать всё вручную:

### 1. Сборка libvis.so

```bash
cd /home/engine/project/mod_menu_rootless/jni
$ANDROID_NDK_HOME/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk
```

Результат: `libs/arm64-v8a/libvis.so` и `libs/armeabi-v7a/libvis.so`

### 2. Сборка classes.dex

```bash
cd /home/engine/project/mod_menu_rootless

# Компиляция Java
mkdir -p build/classes
javac -d build/classes -source 1.8 -target 1.8 \
    -bootclasspath $ANDROID_HOME/platforms/android-30/android.jar \
    java/com/modmenu/loader/*.java

# Создание DEX
$ANDROID_HOME/build-tools/*/d8 \
    --lib $ANDROID_HOME/platforms/android-30/android.jar \
    --release \
    --output build/ \
    build/classes/com/modmenu/loader/*.class

mv build/classes.dex build/classes.dex
```

Результат: `build/classes.dex`

### 3. Копирование в APK

```bash
# classes.dex
mkdir -p /home/engine/project/assets
cp build/classes.dex /home/engine/project/assets/

# libvis.so
mkdir -p /home/engine/project/lib/arm64-v8a
cp libs/arm64-v8a/libvis.so /home/engine/project/lib/arm64-v8a/

mkdir -p /home/engine/project/lib/armeabi-v7a
cp libs/armeabi-v7a/libvis.so /home/engine/project/lib/armeabi-v7a/
```

## Содержимое мод меню

### Java Loader (classes.dex)

- `ModMenuLoader` - Главный класс загрузчика
- `DexInjector` - Инъектор DEX в classloader
- `OverlayService` - UI overlay меню
- `LifecycleListener` - Отслеживание lifecycle приложения

### Native Library (libvis.so)

- **ESP**: Box, Skeleton, Health, Distance, Name
- **Aimbot**: FOV, Smoothing, Visible Only, Team Check
- **Protection**: Anti-debug, Anti-emulator, Anti-Frida, Integrity checks

## Troubleshooting

### Ошибка: ANDROID_NDK_HOME not set

```bash
export ANDROID_NDK_HOME=/path/to/your/ndk
export ANDROID_HOME=/path/to/your/sdk
```

### Ошибка: d8 tool not found

Убедитесь, что `ANDROID_HOME` указывает на директорию с build-tools.

### Ошибка при сборке native

Проверьте, что у вас установлены необходимые инструменты:
```bash
sudo apt-get install build-essential cmake
```

### APK не устанавливается

Убедитесь, что APK подписан:
```bash
apksigner verify standoff2_modded.apk
```

### Мод меню не появляется

1. Проверьте логи через `adb logcat | grep ModMenu`
2. Убедитесь, что разрешено наложение поверх окон
3. Проверьте, что инъекция была выполнена в правильном классе Application

## Файлы проекта

После успешной сборки и инъекции:

```
/home/engine/project/
├── assets/
│   └── classes.dex          <-- Java мод меню
├── lib/
│   ├── arm64-v8a/
│   │   └── libvis.so        <-- Native мод меню (ARM64)
│   └── armeabi-v7a/
│       └── libvis.so        <-- Native мод меню (ARM32)
├── AndroidManifest.xml
├── classes.dex              <-- Оригинальный DEX
├── classes2.dex
└── ... (остальные файлы APK)
```

## Авторы

Этот мод меню создан для образовательных целей.
