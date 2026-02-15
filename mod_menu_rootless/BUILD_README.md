# 🔧 Сборка и Интеграция Мода

## 📋 Подготовка

### 1. Установите зависимости:

**Android NDK** (для сборки C++):
```bash
# Скачайте с https://developer.android.com/ndk/downloads
# Пример для Linux:
wget https://dl.google.com/android/repository/android-ndk-r25c-linux.zip
unzip android-ndk-r25c-linux.zip
export ANDROID_NDK_HOME=$PWD/android-ndk-r25c
```

**Android SDK** (для Java):
```bash
# Установите Android Studio или cmdline-tools
export ANDROID_HOME=/opt/android-sdk
```

**Дополнительные инструменты:**
```bash
# apktool (для декомпиляции APK)
wget https://bitbucket.org/iBotPeaches/apktool/downloads/apktool_2.9.1.jar
sudo mv apktool_2.9.1.jar /usr/local/bin/apktool.jar

# Создайте wrapper:
echo '#!/bin/bash
java -jar /usr/local/bin/apktool.jar "$@"' | sudo tee /usr/local/bin/apktool
sudo chmod +x /usr/local/bin/apktool
```

### 2. Получите APK Standoff 2:
```bash
# Скачайте APK с вашего устройства
adb pull /data/app/com.axlebolt.standoff2-*/base.apk standoff2.apk
```

---

## 🚀 Быстрый старт

### Вариант 1: Полный автоматический процесс
```bash
cd mod_menu_rootless
export ANDROID_NDK_HOME=/path/to/ndk
export ANDROID_HOME=/path/to/sdk
./full_build.sh /path/to/standoff2.apk
```

### Вариант 2: По шагам
```bash
cd mod_menu_rootless
export ANDROID_NDK_HOME=/path/to/ndk
export ANDROID_HOME=/path/to/sdk

# Шаг 1: Сборка
./build.sh

# Шаг 2: Интеграция
./inject_apk.sh /path/to/standoff2.apk
```

---

## 📦 Что происходит при сборке

### build.sh выполняет:
1. **Сборка native library** (`libv.so`)
   - Компиляция C++ кода через ndk-build
   - ARM64 и ARM32 архитектуры
   - Стрип символов для безопасности

2. **Сборка Java классов**
   - Компиляция обфусцированных классов (a.b.c.*)
   - Создание DEX файла (a.dex)

3. **Упаковка**
   - `output/lib/arm64-v8a/libv.so`
   - `output/lib/armmeabi-v7a/libv.so`
   - `output/a.dex`

### inject_apk.sh выполняет:
1. **Извлечение APK**
2. **Интеграция библиотек** в `lib/[arch]/`
3. **Интеграция DEX** в `assets/a`
4. **Патчинг AndroidManifest.xml**:
   - Добавление `SYSTEM_ALERT_WINDOW` разрешения
   - Регистрация сервиса `a.b.c.e`
5. **Патчинг Application класса**:
   - Вставка вызовов `a.b.c.f.inject()` и `a.b.c.d.inject()`
6. **Выравнивание и подпись APK**

---

## 🎯 Результат

```
standoff2_modded_signed.apk  ← Готовый модифицированный APK
```

**Установка:**
```bash
adb install -r standoff2_modded_signed.apk
```

**При первом запуске:**
1. Запустите Standoff 2
2. Разрешите "Отображение поверх других приложений" в настройках
3. Нажмите кнопку "M" в игре для открытия меню

---

## 🔧 Ручная сборка (если скрипты не работают)

### Native библиотека:
```bash
cd jni
$ANDROID_NDK_HOME/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk
```

### Java классы:
```bash
mkdir -p build/classes
javac -d build/classes -source 1.8 -target 1.8 \
    -bootclasspath $ANDROID_HOME/platforms/android-30/android.jar \
    java/a/b/c/*.java
```

### DEX файл:
```bash
$ANDROID_HOME/build-tools/30.0.3/d8 \
    --lib $ANDROID_HOME/platforms/android-30/android.jar \
    --release --output build/ build/classes/a/b/c/*.class
mv build/classes.dex build/a.dex
```

---

## 🛠️ Устранение неполадок

### Ошибка: "ANDROID_NDK_HOME not set"
**Решение:** Установите переменную окружения:
```bash
export ANDROID_NDK_HOME=/opt/android-ndk-r25c
export ANDROID_HOME=/opt/android-sdk
```

### Ошибка: "d8: command not found"
**Решение:** Установите build-tools через SDK Manager:
```bash
$ANDROID_HOME/cmdline-tools/latest/bin/sdkmanager "build-tools;30.0.3"
```

### Ошибка: "apktool not found"
**Решение:** Установите apktool (см. раздел подготовки)

### Ошибка: "No space left on device"
**Решение:** Очистите временные файлы:
```bash
rm -rf build/ output/ apk_work/ ~/.cache/
```

---

## 📂 Структура выходных файлов

```
mod_menu_rootless/
 output/
   ├── lib/
   │   ├── arm64-v8a/libv.so      ← ARM64 native lib
   │   └── armeabi-v7a/libv.so    ← ARM32 native lib
   └── a.dex                       ← Java classes DEX
 standoff2_modded_signed.apk    ← Готовый APK (после inject)
 debug.keystore                 ← Подпись (автосоздание)
```

---

## ⚠️ Важные замечания

1. **IL2CPP offsets** уже настроены в коде (вы подтвердили, что они верны)

2. **Обфускация активна:**
   - Классы: `a.b.c.d`, `a.b.c.e`...
   - Библиотека: `libv.so` (не `libmodmenu.so`)
   - DEX: `a` (не `modmenu.dex`)

3. **Логи удалены** - мод работает silently

4. **ESP работает** через OpenGL hook

5. **Подпись APK изменится** - это нормально для модифицированного APK

---

## 🎮 Использование мода

- Кнопка **M** - открыть/закрыть меню
- **E** - вкл/выкл ESP
- **B** - ESP Box
- **S** - ESP Skeleton
- **A** - Aimbot
- **FOV слайдер** - настройка угла

---

## 📞 Поддержка

1. Проверьте переменные окружения
2. Убедитесь, что apktool установлен
3. Проверьте версию NDK (r21+ рекомендуется)
4. Убедитесь, что APK не поврежден

