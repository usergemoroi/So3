# Сводка по сборке мод меню (classes.dex + libvis.so)

## Что было сделано

✅ Подготовлены скрипты для автоматической сборки
✅ Изменено название библиотеки с `modmenu_noroot` на `vis`
✅ Созданы инструкции и руководства

## Структура файлов проекта

### Исходный код
- `mod_menu_rootless/java/` - Java исходники
- `mod_menu_rootless/jni/` - C++ исходники

### Скрипты сборки
- `mod_menu_rootless/build_vis.sh` - Полная сборка (Java + Native)
- `mod_menu_rootless/inject_to_apk.sh` - Инъекция в распакованный APK
- `mod_menu_rootless/build_and_inject_all.sh` - Полный процесс
- `mod_menu_rootless/build_java_only.sh` - Только Java (если нет NDK)

### Документация
- `mod_menu_rootless/README_VIS.md` - Основная инструкция
- `mod_menu_rootless/SETUP_GUIDE.md` - Руководство по установке Android SDK/NDK

## Текущее состояние системы

### Установлено
✅ Java JDK 17.0.18 (javac)
✅ Build-essential

### Не установлено
❌ Android NDK
❌ Android SDK с build-tools

## Как собрать проект

### Способ 1: Полная автоматическая сборка (рекомендуется)

**Требует:** Android SDK + NDK

```bash
# 1. Установите Android SDK и NDK (см. SETUP_GUIDE.md)
export ANDROID_HOME=/path/to/android-sdk
export ANDROID_NDK_HOME=/path/to/android-ndk

# 2. Запустите полный скрипт
cd /home/engine/project/mod_menu_rootless
./build_and_inject_all.sh
```

### Способ 2: По шагам

```bash
cd /home/engine/project/mod_menu_rootless

# Сборка libvis.so
cd jni
$ANDROID_NDK_HOME/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk
cd ..

# Сборка classes.dex
mkdir -p build/classes
javac -d build/classes -source 1.8 -target 1.8 \
    -bootclasspath $ANDROID_HOME/platforms/android-30/android.jar \
    java/com/modmenu/loader/*.java

$ANDROID_HOME/build-tools/*/d8 \
    --lib $ANDROID_HOME/platforms/android-30/android.jar \
    --release \
    --output build/ \
    build/classes/com/modmenu/loader/*.class

# Инъекция в APK
./inject_to_apk.sh
```

### Способ 3: Только Java (без native)

**Требует:** Только Android SDK (без NDK)

```bash
cd /home/engine/project/mod_menu_rootless
./build_java_only.sh

# Скопировать результат
cp build/classes.dex /home/engine/project/assets/
```

## Результаты сборки

После успешной сборки файлы будут в:

```
/home/engine/project/
├── assets/
│   └── classes.dex          <-- Java мод меню
├── lib/
│   ├── arm64-v8a/
│   │   └── libvis.so        <-- Native мод (ARM64)
│   └── armeabi-v7a/
│       └── libvis.so        <-- Native мод (ARM32)
└── INJECTION_INFO.txt        <-- Информация по инъекции
```

## Интеграция в APK

### Если APK уже распакован (как в этом проекте)

```bash
cd /home/engine/project

# Файлы уже скопированы после ./inject_to_apk.sh

# Пересобрать APK
apktool b . -o standoff2_modded.apk

# Подписать
apksigner sign --ks debug.keystore --ks-pass pass:android standoff2_modded.apk

# Установить
adb install -r standoff2_modded.apk
```

### Важно: Ручная интеграция кода

После сборки APK нужно внедрить вызовы загрузчика в главный Application класс:

1. Найдите файл `smali/.../YourApplication.smali`
2. В методе `onCreate()` после `invoke-super {p0}, ...onCreate` добавьте:

```smali
invoke-static {p0}, Lcom/modmenu/loader/DexInjector;->inject(Landroid/content/Context;)V
invoke-static {p0}, Lcom/modmenu/loader/ModMenuLoader;->inject(Landroid/app/Application;)V
```

3. Убедитесь, что в AndroidManifest.xml есть разрешения:
```xml
<uses-permission android:name="android.permission.SYSTEM_ALERT_WINDOW"/>
<uses-permission android:name="android.permission.INTERNET"/>
```

## Функционал мод меню

### Java Loader (classes.dex)
- `ModMenuLoader` - Загрузка native библиотеки и запуск
- `DexInjector` - Инъекция DEX в classloader приложения
- `OverlayService` - UI overlay с настройками
- `LifecycleListener` - Отслеживание жизненного цикла

### Native Library (libvis.so)
- **ESP**: Box, Skeleton, Health, Distance, Name ESP
- **Aimbot**: FOV, Smoothing, Visible check, Team check
- **Protection**: Anti-debug, Anti-emulator, Anti-Frida, Integrity checks

## Быстрый чек-лист

- [ ] Установить Android SDK
- [ ] Установить Android NDK
- [ ] Установить переменные окружения ANDROID_HOME и ANDROID_NDK_HOME
- [ ] Запустить `./build_and_inject_all.sh`
- [ ] Проверить наличие файлов в `/home/engine/project/assets/` и `/home/engine/project/lib/`
- [ ] Добавить вызовы инъекции в Application класс
- [ ] Пересобрать APK с apktool
- [ ] Подписать APK
- [ ] Установить на устройство

## Troubleshooting

### Ошибка: d8 not found
Установите build-tools: `sdkmanager "build-tools;30.0.3"`

### Ошибка: ndk-build not found
Установите NDK: `sdkmanager "ndk;25.2.9519653"`

### Java compilation failed
Проверьте версию Java: `javac -version` (должна быть 8+)

### Native build failed
Проверьте NDK версию и установку переменной ANDROID_NDK_HOME

## Дополнительные ресурсы

- Android SDK Downloads: https://developer.android.com/studio
- Android NDK Guide: https://developer.android.com/ndk/guides
- APK Tool: https://ibotpeaches.github.io/Apktool/

## Контакт и поддержка

Смотрите файлы в `mod_menu_rootless/` для более подробной информации:
- `README_VIS.md` - Основная инструкция
- `SETUP_GUIDE.md` - Установка SDK/NDK
- `FEATURES.md` - Описание функций
- `CHANGELOG.md` - История изменений
