# Mod Menu Rootless - Build Instructions

## Текущий статус


### 1. Созданные файлы:
- `/mod_menu_rootless/jni/config.hpp` - Общая структура конфигурации ModConfig
- `/mod_menu_rootless/quick_build.sh` - Автоматический скрипт сборки, который скачивает все необходимое

### 2. Исправленные проблемы:
- Добавлен `config.hpp` для решения проблемы forward declaration
- Обновлены `esp_noroot.hpp` и `aimbot_noroot.hpp` для использования полного определения ModConfig
- Обновлен `main_noroot.cpp` для использования нового файла config.hpp

### 3. Установленные инструменты:
- Android NDK r21e (уже скачан в `/mod_menu_rootless/tools_cache/android-ndk-r21e/`)
- OpenJDK 11 (установлен в системе)

## Известные проблемы при сборке

1. **Проблема с линковкой C++ стандартной библиотеки**:
   Ошибки вида `undefined reference to std::__ndk1::thread`

   **Решение**: В `Android.mk` добавлено `LOCAL_STATIC_LIBRARIES := c++_static`

2. **Проблема с символами `__executable_start` и `etext`**:
   Эти символы недоступны при стандартной линковке Android NDK

   **Решение**: Необходимо закомментировать вызовы `MemoryIntegrity::Initialize()` и 
   `MemoryIntegrity::CheckIntegrity()` в файле `main_noroot.cpp`, либо полностью 
   переписать эти методы для использования `/proc/self/maps`.

## Как собрать проект (упрощенная версия)

### Шаг 1: Исправьте проблемы с MemoryIntegrity

```cpp
// Protection::MemoryIntegrity::Initialize();  // Строка ~55
// И в CheckProtection():
// Protection::MemoryIntegrity::CheckIntegrity();
```

### Шаг 2: Исправьте Android.mk

```makefile
LOCAL_STATIC_LIBRARIES := c++_static
```

### Шаг 3: Соберите нативную библиотеку

```bash
cd /home/engine/project/mod_menu_rootless/jni
/home/engine/project/mod_menu_rootless/tools_cache/android-ndk-r21e/ndk-build \
    NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk
```

### Шаг 4: Скомпилируйте Java классы

```bash
cd /home/engine/project/mod_menu_rootless
mkdir -p build/classes

javac -d build/classes \
    -source 1.8 -target 1.8 \
    -bootclasspath /usr/lib/jvm/java-11-openjdk-amd64/jre/lib/rt.jar \
    java/com/modmenu/loader/*.java
```

### Шаг 5: Создайте DEX файл


```bash
# Используя d8
d8 --lib $ANDROID_HOME/platforms/android-28/android.jar \
   --release \
   --output build/ \
   build/classes/com/modmenu/loader/*.class

mv build/classes.dex build/modmenu.dex
```

### Шаг 6: Скопируйте файлы в проект APK

```bash
# Нативная библиотека
mkdir -p /path/to/apk/lib/arm64-v8a/
cp jni/libs/arm64-v8a/libmodmenu_noroot.so /path/to/apk/lib/arm64-v8a/

# DEX файл
mkdir -p /path/to/apk/assets/
cp build/modmenu.dex /path/to/apk/assets/
```

## Альтернативный метод: Использование готовой библиотеки


1. Собрать только Java часть (DEX)
2. Использовать предварительно скомпилированную нативную библиотеку

## Проверка сборки

- `jni/libs/arm64-v8a/libmodmenu_noroot.so`
- `jni/libs/armeabi-v7a/libmodmenu_noroot.so` (опционально)
- `build/modmenu.dex`

## Дополнительная информация

- `/mod_menu_rootless/README.md`
- `/mod_menu_rootless/INTEGRATION_GUIDE.md`
- `/mod_menu_rootless/FINAL_BUILD_GUIDE.md`

