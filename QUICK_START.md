# 🚀 Быстрый старт - Rootless Mod Menu

## 📦 За 5 минут до готового APK

### Шаг 1: Установите зависимости

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y openjdk-11-jdk python3 git wget unzip

# Скачайте Android NDK (если нет)
wget https://dl.google.com/android/repository/android-ndk-r25c-linux.zip
unzip android-ndk-r25c-linux.zip
export ANDROID_NDK_HOME=$PWD/android-ndk-r25c

# Скачайте Android SDK command line tools
wget https://dl.google.com/android/repository/commandlinetools-linux-9477386_latest.zip
mkdir -p android-sdk/cmdline-tools
unzip commandlinetools-linux-9477386_latest.zip -d android-sdk/cmdline-tools
mv android-sdk/cmdline-tools/cmdline-tools android-sdk/cmdline-tools/latest
export ANDROID_HOME=$PWD/android-sdk

# Установите build-tools
$ANDROID_HOME/cmdline-tools/latest/bin/sdkmanager --sdk_root=$ANDROID_HOME "build-tools;33.0.0" "platforms;android-33"

# Установите apktool
wget https://raw.githubusercontent.com/iBotPeaches/Apktool/master/scripts/linux/apktool
wget https://bitbucket.org/iBotPeaches/apktool/downloads/apktool_2.8.1.jar
chmod +x apktool
sudo mv apktool /usr/local/bin/
sudo mv apktool_2.8.1.jar /usr/local/bin/
```

### Шаг 2: Соберите мод

```bash
cd mod_menu_rootless
./build.sh
```

Вы увидите:
```
==========================================
  Rootless Mod Menu - Build Script
==========================================

[1/4] Building native library...
...
[2/4] Compiling Java classes...
...
[3/4] Creating DEX file...
...
[4/4] Packaging...
...

==========================================
  Build complete!
==========================================

Output files:
  output/libmodmenu_noroot.so (ARM64)
  output/modmenu.dex
```

### Шаг 3: Запатчите APK

```bash
cd tools

# ВАРИАНТ A: Автоматический патчинг (рекомендуется)
python3 apk_patcher.py /path/to/standoff2.apk

# ВАРИАНТ B: С быстрой установкой
./install.sh /path/to/standoff2.apk
```

Процесс:
```
==========================================
  APK Patcher - Rootless Mod Menu
==========================================

[*] Extracting APK...
[+] APK extracted successfully
[*] Patching AndroidManifest.xml...
[+] Manifest patched successfully
[*] Injecting mod loader into DEX...
[+] Loader injected successfully
[*] Copying mod files...
[+] Copied modmenu.dex
[+] Copied ARM64 library
[*] Rebuilding APK...
[+] APK rebuilt successfully
[*] Signing APK...
[+] APK signed: standoff2_modded_signed.apk

==========================================
  Patching complete!
==========================================
```

### Шаг 4: Установите на устройство

```bash
# Подключите Android устройство через USB
# Включите "Отладка по USB" в настройках разработчика

# Проверьте подключение
adb devices

# Установите
adb install standoff2_modded_signed.apk
```

### Шаг 5: Запустите и играйте!

1. **Откройте Standoff 2**

2. **Разрешите overlay** (при первом запуске)
   - Появится запрос: "Разрешить отображение поверх других окон?"
   - Нажмите "Настройки" → Включите переключатель
   - Вернитесь в игру

3. **Найдите кнопку меню**
   - В левом верхнем углу появится кнопка "Menu"
   - Можете перетаскивать её куда хотите

4. **Откройте меню**
   - Нажмите на кнопку "Menu"
   - Откроется полное меню с настройками

5. **Настройте функции**
   ```
   ESP Settings:
   ☑ Enable ESP ............... Включить ESP
   ☑ Box ESP .................. Рамки
   ☑ Skeleton ESP ............. Скелет
   ☑ Health ESP ............... Здоровье
   
   Aimbot Settings:
   ☑ Enable Aimbot ............ Включить aimbot
   ☑ Visible Only ............. Только видимые
   FOV: [========] 90° ........ Угол обзора
   Smooth: [===] 5.0 .......... Плавность
   ```

6. **Играйте!** 🎮

## 🔧 Решение проблем

### Проблема: "Устройство не найдено"

```bash
# Проверьте USB отладку
adb devices

# Если пусто, включите USB отладку:
# Настройки → О телефоне → 7 раз нажать на "Номер сборки"
# Настройки → Для разработчиков → USB отладка → Включить
```

### Проблема: "BUILD FAILED"

```bash
# Проверьте переменные окружения
echo $ANDROID_NDK_HOME
echo $ANDROID_HOME

# Если пустые, установите снова:
export ANDROID_NDK_HOME=/path/to/ndk
export ANDROID_HOME=/path/to/sdk
```

### Проблема: "Меню не появляется"

```bash
# 1. Проверьте логи
adb logcat | grep ModMenu

# 2. Убедитесь что разрешение дано
adb shell appops get com.axlebolt.standoff2 SYSTEM_ALERT_WINDOW

# 3. Дайте разрешение вручную
adb shell appops set com.axlebolt.standoff2 SYSTEM_ALERT_WINDOW allow
```

### Проблема: "Игра крашится"

```bash
# Посмотрите логи краша
adb logcat | grep "FATAL\|AndroidRuntime"

# Возможные причины:
# - Неправильная архитектура (проверьте ARM64 vs ARM32)
# - Эмулятор (используйте реальное устройство)
# - Устаревшая версия игры (обновите оффсеты)
```

## 📊 Проверка совместимости устройства

```bash
cd mod_menu_rootless/tools
./test_device.sh
```

Вывод:
```
==========================================
  Device Compatibility Test
==========================================

Device information:
  Model: SM-G981B
  Brand: samsung
  Android Version: 12
  SDK Level: 31
  Architecture: arm64-v8a

Compatibility check:
  ✅ Android version compatible (API 31)
  ✅ ARM64 architecture detected
  ✅ Overlay permission supported
  📱 Device is NOT ROOTED (perfect for rootless mod!)
  ✅ Real device detected

==========================================
  Device is COMPATIBLE! ✅
==========================================
```

## 🐛 Отладка в реальном времени

```bash
cd mod_menu_rootless/tools
./debug_logs.sh
```

Вывод:
```
==========================================
  Real-time Mod Debug Logs
==========================================

Press Ctrl+C to stop

02-15 12:34:56.789 12345 12346 I ModMenuNoRoot: Library loaded
02-15 12:34:56.790 12345 12346 I ModMenuNoRoot: JNI_OnLoad called
02-15 12:34:56.791 12345 12346 I DexInjector: DEX injected successfully!
02-15 12:34:56.792 12345 12346 I ModMenuLoader: Mod menu loaded successfully!
02-15 12:34:56.793 12345 12346 I OverlayService: Overlay created
02-15 12:34:56.794 12345 12346 I IL2CPP_NoRoot: libil2cpp.so base: 0x7123456000
02-15 12:34:56.795 12345 12346 I Hooks_NoRoot: Hooks installed successfully
...
```

## 💡 Советы

### Безопасность

✅ **Делайте:**
- Используйте на тестовом аккаунте
- Держите настройки умеренными
- Играйте в casual режимах

❌ **Не делайте:**
- Не используйте в ranked
- Не ставьте экстремальные значения
- Не стримьте с модом

### Производительность

- **Отключайте ненужные функции** - меньше нагрузка на CPU
- **Уменьшите FOV** - меньше проверок = выше FPS
- **Закройте другие приложения** - больше RAM для игры

### Обновления

После обновления игры:

```bash
# 1. Пересоберите мод
cd mod_menu_rootless
./build.sh

# 2. Перепатчите новый APK
cd tools
python3 apk_patcher.py /path/to/standoff2_new.apk

# 3. Переустановите
adb install -r standoff2_modded_signed.apk
```

## 📱 Поддерживаемые устройства

### ✅ Полностью работает

- Samsung Galaxy S/Note series (S10+)
- OnePlus 7+
- Xiaomi Mi/Poco (9+)
- Google Pixel (4+)
- Realme (6+)
- ASUS ROG Phone

### ⚠️ Ограниченная поддержка

- Huawei (без Google Services)
- Устройства с Android < 10
- Бюджетные устройства (<2GB RAM)

### ❌ Не поддерживается

- iOS (iPhone/iPad)
- Эмуляторы (BlueStacks, Nox, etc.)
- Android < 7.0

## 🎓 Дальнейшее чтение

- 📘 [Полное руководство](INTEGRATION_GUIDE.md)
- ✨ [Список всех функций](FEATURES.md)
- 📝 [История изменений](CHANGELOG.md)
- 🏗️ [Техническая документация](ROOTLESS_SUMMARY.md)

## 🤝 Нужна помощь?

1. **Документация** - Прочитайте README.md
2. **FAQ** - Смотрите INTEGRATION_GUIDE.md
3. **Issues** - GitHub Issues
4. **Community** - Discord / Telegram

---

## ✅ Чеклист установки

- [ ] Установлены зависимости (NDK, SDK, apktool)
- [ ] Собран мод (`./build.sh`)
- [ ] APK запатчен (`apk_patcher.py`)
- [ ] Устройство подключено (`adb devices`)
- [ ] APK установлен (`adb install`)
- [ ] Разрешение overlay дано
- [ ] Меню появилось в игре
- [ ] Функции работают

Если все ✅ - **Поздравляем! Вы готовы!** 🎉

---

**Время установки:** ~5-10 минут  
**Сложность:** ⭐⭐☆☆☆ (Легко)  
**Требуется root:** ❌ НЕТ!  

**Удачной игры!** 🎮
