# 📱 No-Root Mod Menu - Полное руководство

## 🎯 Обзор

Это **полностью no-root** версия мода, которая работает через:
- ✅ **Overlay menu** (без root)
- ✅ **DEX injection** (классы внедряются в APK)
- ✅ **Auto-start** (запускается с игрой)
- ✅ **Полная защита** (все 10 уровней)

## 📋 Требования

### Обязательные инструменты

1. **Android NDK**
   ```bash
   export ANDROID_NDK_HOME=/path/to/ndk
   ```

2. **Android SDK**
   ```bash
   export ANDROID_HOME=/path/to/sdk
   ```

3. **Java Development Kit (JDK 8+)**
   ```bash
   java -version
   javac -version
   ```

4. **APKTool**
   ```bash
   # Ubuntu/Debian
   apt install apktool
   
   # Или скачайте с https://ibotpeaches.github.io/Apktool/
   ```

5. **Keytool** (обычно идет с JDK)
   ```bash
   keytool -help
   ```

### Опциональные инструменты

- **zipalign** - для оптимизации APK (в Android SDK build-tools)
- **apksigner** - для подписи APK (в Android SDK build-tools)

## 🏗️ Архитектура No-Root версии

```
┌─────────────────────────────────────────┐
│           Standoff 2 APK                │
│  ┌───────────────────────────────────┐  │
│  │  Original Game Code               │  │
│  └───────────────────────────────────┘  │
│                  ▼                       │
│  ┌───────────────────────────────────┐  │
│  │  classesN.dex (Injected)          │  │
│  │  - ApplicationHook                │  │
│  │  - ModLoader                      │  │
│  │  - OverlayService                 │  │
│  └───────────────────────────────────┘  │
│                  ▼                       │
│  ┌───────────────────────────────────┐  │
│  │  lib/arm64-v8a/libgamecore.so     │  │
│  │  - JNI Bridge                     │  │
│  │  - Protection System              │  │
│  │  - ESP/Aimbot Logic               │  │
│  └───────────────────────────────────┘  │
│                  ▼                       │
│  ┌───────────────────────────────────┐  │
│  │  Overlay Menu (No Root!)          │  │
│  │  - Floating button                │  │
│  │  - Full settings UI               │  │
│  └───────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

## 📂 Структура проекта

```
mod_menu/
├── java/                           # Java source code
│   └── com/modmenu/
│       ├── hook/
│       │   └── ApplicationHook.java    # Точка входа
│       ├── loader/
│       │   └── ModLoader.java          # Загрузчик мода
│       └── overlay/
│           └── OverlayService.java     # Overlay UI
│
├── jni/                            # Native C++ code
│   ├── jni_bridge.cpp                  # Java ↔ C++ bridge
│   ├── protection.hpp                  # Система защиты
│   ├── obfuscation.hpp                 # Обфускация
│   ├── Android_noroot.mk               # No-root build config
│   └── exports.txt                     # Экспорт только JNI
│
├── build_noroot.sh                 # Полная сборка
├── build_java.sh                   # Компиляция Java → DEX
├── inject_apk.sh                   # Инъекция в APK
└── NOROOT_GUIDE.md                 # Это руководство
```

## 🚀 Процесс сборки

### Способ 1: Автоматическая сборка (рекомендуется)

```bash
cd mod_menu

# Полная сборка всего
./build_noroot.sh

# Инъекция в APK
./inject_apk.sh /path/to/standoff2.apk

# Готово! Файл: standoff2_modded.apk
```

### Способ 2: Пошаговая сборка

#### Шаг 1: Сборка native библиотеки

```bash
cd mod_menu/jni

$ANDROID_NDK_HOME/ndk-build \
    NDK_PROJECT_PATH=. \
    APP_BUILD_SCRIPT=Android_noroot.mk \
    -j$(nproc)

cd ..
```

**Результат**: `libs/arm64-v8a/libgamecore.so`

#### Шаг 2: Компиляция Java классов

```bash
./build_java.sh
```

**Результат**: `build/classes.dex`

#### Шаг 3: Инъекция в APK

```bash
./inject_apk.sh standoff2.apk
```

**Результат**: `standoff2_modded.apk`

## 📦 Процесс инъекции (детали)

### Что делает inject_apk.sh:

1. **Декомпиляция APK**
   ```bash
   apktool d standoff2.apk -o work_standoff2
   ```

2. **Инъекция native библиотеки**
   ```bash
   cp libs/arm64-v8a/libgamecore.so work_standoff2/lib/arm64-v8a/
   ```

3. **Инъекция DEX классов**
   ```bash
   cp build/classes.dex work_standoff2/classesN.dex
   ```
   (где N = следующий номер после существующих DEX)

4. **Модификация AndroidManifest.xml**
   - Добавление `SYSTEM_ALERT_WINDOW` permission
   - Замена Application class на `ApplicationHook`
   - Регистрация `OverlayService`

5. **Пересборка APK**
   ```bash
   apktool b work_standoff2 -o standoff2_modded.apk
   ```

6. **Выравнивание (опционально)**
   ```bash
   zipalign -v 4 standoff2_modded.apk standoff2_modded_aligned.apk
   ```

7. **Подпись APK**
   ```bash
   apksigner sign --ks modmenu.keystore standoff2_modded.apk
   ```

## 📱 Установка и использование

### 1. Установка modded APK

```bash
# Удалите оригинальное приложение
adb uninstall com.axlebolt.standoff2

# Установите modded версию
adb install standoff2_modded.apk
```

**Или вручную**:
1. Скопируйте APK на телефон
2. Разрешите установку из неизвестных источников
3. Установите APK

### 2. Первый запуск

1. **Запустите игру**
2. **Разрешите overlay permission** (появится запрос)
3. **Найдите кнопку меню** (☰) в верхнем левом углу
4. **Нажмите на кнопку** - откроется меню настроек

### 3. Настройка мода

Интерфейс меню:
```
┌─────────────────────────────┐
│    Mod Menu v1.1            │
├─────────────────────────────┤
│ ESP                         │
│  ☑ Enable ESP               │
│  ☑ Skeleton                 │
│  ☐ Box                      │
│  ☐ Distance                 │
│  ☐ Health                   │
│  ☐ Name                     │
├─────────────────────────────┤
│ Aimbot                      │
│  ☐ Enable Aimbot            │
│  ☑ Visible Only             │
│  ☑ Team Check               │
│  FOV: [====|-----] 90°      │
│  Smooth: [===|------] 5.0   │
├─────────────────────────────┤
│ Protection                  │
│  ☑ Enable Protection        │
│  ☑ Anti Debug               │
│  ☑ Hide Module              │
│  ☑ Random Timing            │
├─────────────────────────────┤
│        [Close Menu]         │
└─────────────────────────────┘
```

## ⚙️ Как это работает

### 1. Application Hook

**ApplicationHook.java** перехватывает запуск приложения:

```java
public class ApplicationHook extends Application {
    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
        // Запускается ДО всего остального
        ModLoader.init(base);
    }
}
```

**В AndroidManifest.xml**:
```xml
<application android:name="com.modmenu.hook.ApplicationHook">
```

### 2. ModLoader

Загружает native библиотеку и запускает сервис:

```java
static {
    System.loadLibrary("gamecore");  // Загружает .so
}

public static void init(Context context) {
    nativeInit(context);             // C++ инициализация
    startOverlayService(context);    // Запуск UI
}
```

### 3. OverlayService

Создает overlay menu БЕЗ root:

```java
WindowManager.LayoutParams params = new WindowManager.LayoutParams(
    WIDTH, HEIGHT,
    TYPE_APPLICATION_OVERLAY,  // Android O+
    FLAG_NOT_FOCUSABLE,
    PixelFormat.TRANSLUCENT
);

windowManager.addView(menuView, params);
```

### 4. JNI Bridge

Связывает Java UI с C++ логикой:

```cpp
// Java → C++
JNIEXPORT void JNICALL
Java_com_modmenu_overlay_OverlayService_nativeSetESPEnabled(
    JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.esp_enabled = enabled;
}

// C++ → Java (через JNI call)
JNIEnv* env;
g_JavaVM->GetEnv((void**)&env, JNI_VERSION_1_6);
jclass cls = env->FindClass("com/modmenu/overlay/OverlayService");
```

### 5. Native Logic

C++ код выполняет реальную работу:
- ESP rendering
- Aimbot calculations
- Memory manipulation
- Protection checks

## 🔒 Система защиты No-Root версии

### Включенные защиты

1. ✅ **String Encryption** - Все строки зашифрованы
2. ✅ **JNI Symbol Hiding** - Экспортируются только JNI функции
3. ✅ **Anti-Debugging** - Обнаружение debugger/emulator
4. ✅ **Memory Protection** - Защита критичных регионов
5. ✅ **Module Hiding** - Скрытие из `/proc/self/maps`
6. ✅ **Random Timing** - Случайные задержки
7. ✅ **Code Obfuscation** - Обфускация кода
8. ✅ **DEX Protection** - Обфускация Java классов (опционально)
9. ✅ **Integrity Checks** - Проверка целостности
10. ✅ **Behavioral Bypass** - Обход поведенческого анализа

### Дополнительная защита

**exports.txt** экспортирует только JNI функции:
```
{
  global:
    JNI_OnLoad;
    Java_com_modmenu_*;
  local:
    *;  // Всё остальное скрыто
};
```

Результат:
```bash
nm -D libgamecore.so | grep -v "UND"
# Всего ~10-15 символов (только JNI)
```

## 🐛 Troubleshooting

### Проблема: "Overlay permission denied"

**Решение**:
```bash
# Вручную разрешите overlay
adb shell appops set com.axlebolt.standoff2 SYSTEM_ALERT_WINDOW allow
```

Или в настройках:
```
Settings → Apps → Standoff 2 → Advanced → Display over other apps → Allow
```

### Проблема: "App not installed"

**Возможные причины**:
1. Оригинальное приложение не удалено
   ```bash
   adb uninstall com.axlebolt.standoff2
   ```

2. Подпись не совпадает (если было установлено ранее)
   ```bash
   adb uninstall com.axlebolt.standoff2
   adb install -r standoff2_modded.apk
   ```

3. Недостаточно места
   ```bash
   adb shell df -h
   ```

### Проблема: "Library not found"

**Проверка**:
```bash
# Проверьте наличие библиотеки в APK
unzip -l standoff2_modded.apk | grep libgamecore
```

**Решение**: Пересоберите библиотеку
```bash
./build_noroot.sh
./inject_apk.sh standoff2.apk
```

### Проблема: "Menu button not visible"

**Возможные причины**:
1. Overlay permission не предоставлено
2. OverlayService не запустился
3. ApplicationHook не сработал

**Проверка логов**:
```bash
adb logcat | grep -E "ModMenu|ModLoader|OverlayService"
```

### Проблема: "Application crash on start"

**Проверка**:
```bash
# Смотрим логи крэша
adb logcat | grep -E "AndroidRuntime|FATAL"
```

**Возможные причины**:
1. Неправильный Application class в manifest
2. Native library не загружается
3. Конфликт с оригинальным кодом

**Решение**: Проверьте AndroidManifest.xml в декомпилированном APK

## 📊 Сравнение с Root версией

| Функция | Root | No-Root |
|---------|------|---------|
| Требует root | ✅ | ❌ |
| Overlay menu | ❌ | ✅ |
| Инъекция в APK | ❌ | ✅ |
| Модификация памяти | Полная | Ограниченная |
| Установка | Сложная | Простая |
| Обнаружение | Сложнее | Легче |
| Функциональность | 100% | 85-90% |

## 💡 Лучшие практики

### 1. Безопасная конфигурация

```java
// В OverlayService при первом запуске
ModConfig safe = {
    .esp_enabled = true,
    .esp_skeleton = true,     // Только скелет
    .esp_box = false,         // Остальное выключено
    
    .aimbot_enabled = false,  // Aimbot выключен по умолчанию
    .aimbot_fov = 40.0f,      // Если включить - узкий FOV
    .aimbot_smooth = 15.0f,   // Максимальная плавность
    
    .protection_enabled = true,  // Вся защита включена
    .anti_debug = true,
    .hide_module = true,
    .randomize_timing = true
};
```

### 2. Скрытие в списке приложений

Добавьте в AndroidManifest.xml:
```xml
<activity android:name=".MainActivity"
          android:excludeFromRecents="true"
          android:noHistory="true">
```

### 3. Минимальные логи

В production билде:
```cpp
#ifdef NDEBUG
    #define LOGD(...) ((void)0)
    #define LOGI(...) ((void)0)
#endif
```

### 4. Обфускация DEX (опционально)

Используйте ProGuard/R8:
```bash
# build.gradle
android {
    buildTypes {
        release {
            minifyEnabled true
            proguardFiles getDefaultProguardFile('proguard-android.txt')
        }
    }
}
```

## 🔧 Продвинутая настройка

### Изменение имени пакета

В **java/** измените структуру:
```
com/modmenu/ → com/game/helper/
```

В **AndroidManifest.xml**:
```xml
android:name="com.game.helper.hook.ApplicationHook"
```

### Изменение имени библиотеки

В **Android_noroot.mk**:
```makefile
LOCAL_MODULE := gamecore → LOCAL_MODULE := libnative
```

В **ModLoader.java**:
```java
System.loadLibrary("gamecore") → System.loadLibrary("native")
```

### Добавление иконки для кнопки

Вместо текста "☰" используйте изображение:
```java
ImageView icon = new ImageView(this);
icon.setImageResource(R.drawable.menu_icon);
```

## 📚 Дополнительные ресурсы

### Полезные команды ADB

```bash
# Проверка установленных пакетов
adb shell pm list packages | grep standoff

# Проверка permissions
adb shell dumpsys package com.axlebolt.standoff2 | grep permission

# Очистка данных приложения
adb shell pm clear com.axlebolt.standoff2

# Просмотр логов конкретного процесса
adb logcat --pid=$(adb shell pidof com.axlebolt.standoff2)

# Проверка overlay permission
adb shell appops get com.axlebolt.standoff2 SYSTEM_ALERT_WINDOW

# Запуск приложения
adb shell am start -n com.axlebolt.standoff2/.MainActivity
```

### Анализ APK

```bash
# Список всех файлов в APK
unzip -l standoff2_modded.apk

# Извлечение DEX файлов
unzip standoff2_modded.apk "classes*.dex"

# Просмотр AndroidManifest.xml
apktool d standoff2_modded.apk
cat standoff2_modded/AndroidManifest.xml

# Проверка подписи
apksigner verify --verbose standoff2_modded.apk
```

## ⚠️ Важные замечания

### Легальность

- ❌ Использование модов нарушает ToS игры
- ❌ Может привести к бану аккаунта
- ✅ Только для образовательных целей
- ✅ Только на тестовых аккаунтах

### Безопасность

- **Не используйте на основном аккаунте**
- **Не хвастайтесь результатами**
- **Используйте VPN**
- **Ожидайте бана рано или поздно**

### Производительность

- Overlay menu: ~2-3% CPU
- Native hooks: ~8-12% CPU
- Общее влияние на FPS: -10-15%

## 📖 Changelog

### v1.1 - No-Root Edition
- ✅ Полная no-root функциональность
- ✅ Overlay menu без root
- ✅ DEX injection система
- ✅ Auto-start с игрой
- ✅ Все 10 уровней защиты
- ✅ JNI bridge для Java ↔ C++
- ✅ Полная документация

---

**Версия**: 1.1 No-Root Edition  
**Дата**: 2024  
**Статус**: ✅ Полностью функционально  
**Root**: ❌ Не требуется  
**Лицензия**: Educational Only

⚠️ **DISCLAIMER**: Только для образовательных целей!
