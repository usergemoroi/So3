# 🚀 Финальное руководство по сборке и установке

## Автоматическая сборка и инъекция в APK

Создан скрипт **auto_build_and_inject.sh**, который автоматически:
1. Собирает нативную библиотеку (с улучшенной защитой)
2. Компилирует Java классы
3. Создает DEX файл
4. Извлекает оригинальный APK
5. Инъектирует модули в APK
6. Патчит AndroidManifest.xml и Application класс
7. Пересобирает и подписывает APK

---

## Использование

### Шаг 1: Подготовка окружения

```bash
# Установите переменные окружения (если еще не установлены)
export ANDROID_NDK_HOME=/path/to/ndk
export ANDROID_HOME=/path/to/sdk

# Убедитесь что установлены:
# - Java JDK 8+
# - Python 3
# - apktool
# - Android SDK build-tools
```

### Шаг 2: Запуск автоматической сборки

```bash
cd mod_menu_rootless

# Одна команда для всего!
./auto_build_and_inject.sh /path/to/standoff2.apk
```

Скрипт выполнит все этапы автоматически.

### Шаг 3: Установка

```bash
# Установите готовый APK
adb install -r standoff2_modded_signed.apk
```

---

## Что было улучшено

### 🛡️ Улучшенная защита (protection_advanced.hpp)

#### 1. Anti-Debug (расширенный)
- ✅ **TracerPid check** - проверка через /proc/self/status
- ✅ **ptrace self-attach** - попытка присоединиться к себе
- ✅ **Process scanning** - поиск gdb, lldb, frida-server
- ✅ **Port scanning** - проверка Frida/IDA портов (23946, 27042, 5039-5041)
- ✅ **Timing checks** - детект замедления от отладчика
- ✅ **Watch thread** - непрерывный мониторинг в фоне

#### 2. Anti-Emulator (расширенный)
- ✅ **File checks** - проверка файлов эмулятора (QEMU, Goldfish, Genymotion, Nox, Droid4X)
- ✅ **Property checks** - анализ system properties (ro.kernel.qemu, ro.hardware, ro.product.*)
- ✅ **CPU analysis** - проверка /proc/cpuinfo на виртуальные характеристики
- ✅ **Sensor checks** - проверка наличия реальных сенсоров

#### 3. Root Detection
- ✅ **SU binary check** - поиск su во всех возможных путях
- ✅ **Root apps detection** - проверка наличия SuperSU, Magisk, KingRoot
- ✅ **Build tags** - проверка test-keys (custom ROM признак)

#### 4. Anti-Frida
- ✅ **Library scanning** - поиск frida-agent, frida-gadget в памяти
- ✅ **Thread analysis** - детект Frida потоков (gmain, gdbus, gum-js-loop)
- ✅ **Continuous monitoring** - постоянная проверка в runtime

#### 5. Memory Integrity
- ✅ **CRC32 checksum** - более надежная проверка чем простая сумма
- ✅ **Code section verification** - проверка от __executable_start до etext
- ✅ **Runtime monitoring** - периодическая проверка целостности

#### 6. String Encryption
- ✅ **Triple XOR** - многоуровневое шифрование
- ✅ **RC4 cipher** - опциональное RC4 шифрование для критичных строк
- ✅ **Position-dependent keys** - ключи зависят от позиции в строке

### 📦 Автоматизация

Скрипт **auto_build_and_inject.sh**:
- Проверяет все зависимости
- Собирает для ARM64 и ARM32
- Автоматически находит Application класс
- Патчит smali код для загрузки мода
- Добавляет нужные разрешения
- Подписывает APK debug ключом

---

## Структура файлов в итоговом APK

```
standoff2_modded_signed.apk
├── AndroidManifest.xml (patched)
│   ├── + SYSTEM_ALERT_WINDOW permission
│   ├── + INTERNET permission
│   └── + OverlayService declaration
│
├── smali/
│   └── [Application class].smali (patched)
│       └── + DexInjector.inject() call
│       └── + ModMenuLoader.inject() call
│
├── assets/
│   └── modmenu.dex ← Java классы мода
│
└── lib/
    ├── arm64-v8a/
    │   └── libmodmenu_noroot.so ← Native библиотека
    └── armeabi-v7a/
        └── libmodmenu_noroot.so (опционально)
```

---

## Как это работает

### 1. Запуск приложения
```
Application.onCreate()
  ↓
DexInjector.inject()
  ├─ Извлекает modmenu.dex из assets
  ├─ Создает DexClassLoader
  └─ Инъектирует в system ClassLoader
  ↓
ModMenuLoader.inject()
  ├─ System.loadLibrary("modmenu_noroot")
  ├─ Запускает OverlayService
  └─ Регистрирует LifecycleListener
```

### 2. Нативная инициализация
```
JNI_OnLoad()
  ↓
MainModThread()
  ├─ Protection::InitProtection()
  │   ├─ Anti-Emulator check → exit if detected
  │   ├─ Anti-Debug check → exit if detected
  │   ├─ Anti-Frida check → exit if detected
  │   ├─ Root Detection → log warning
  │   ├─ Memory Integrity init
  │   └─ Start watch thread
  ↓
  ├─ IL2CPP::Initialize()
  │   ├─ Find libil2cpp.so base address
  │   └─ Setup pattern scanning
  ↓
  ├─ Hooks::InstallHooks()
  │   └─ Place hooks on game functions
  ↓
  └─ Main loop
      ├─ ESP::Update() (if enabled)
      ├─ Aimbot::Update() (if enabled)
      └─ Protection checks every 2 sec
```

### 3. Overlay UI
```
OverlayService.onCreate()
  ├─ Create floating button
  │   └─ Draggable, always on top
  ↓
  └─ Create full menu
      ├─ ESP controls
      ├─ Aimbot controls
      └─ Real-time JNI calls to native
```

---

## Проверка работы

### После установки:

```bash
# 1. Проверьте что APK установлен
adb shell pm list packages | grep standoff

# 2. Запустите игру
adb shell am start -n com.axlebolt.standoff2/.MainActivity

# 3. Проверьте логи
adb logcat | grep -E "ModMenu|Protection|IL2CPP|Overlay"
```

Вы должны увидеть:
```
ModMenuNoRoot: Library loaded
ProtectionAdvanced: Initializing advanced protection...
ProtectionAdvanced: ✅ Protection initialized successfully
IL2CPP_NoRoot: libil2cpp.so base: 0x7...
Hooks_NoRoot: Hooks installed successfully
OverlayService: Overlay created
```

---

## Защита в действии

### Что произойдет при попытке взлома:

**Debugger (gdb/lldb):**
```
ProtectionAdvanced: TracerPid detected: 12345
ProtectionAdvanced: ❌ Debugger detected - Exiting
[Process exits immediately]
```

**Frida:**
```
ProtectionAdvanced: Frida library detected: frida-agent
ProtectionAdvanced: ❌ Frida detected - Exiting
[Process exits immediately]
```

**Emulator:**
```
ProtectionAdvanced: Emulator file detected: /dev/qemu_pipe
ProtectionAdvanced: ❌ Emulator detected - Exiting
[Process exits immediately]
```

**Memory patching:**
```
ProtectionAdvanced: Memory integrity check FAILED!
ProtectionAdvanced: ❌ Memory integrity check failed
[Process exits immediately]
```

---

## Дополнительные опции

### Отключение защиты от эмуляторов (для тестирования)

Отредактируйте `protection_advanced.hpp`:
```cpp
// В функции InitProtection() закомментируйте:
// if (AntiEmulator::IsEmulator()) {
//     LOGD("❌ Emulator detected - Exiting");
//     _exit(0);
// }
```

### Добавление своих проверок

Добавьте в `protection_advanced.hpp`:
```cpp
namespace Protection {
    bool CheckCustom() {
        // Ваша логика
        return false;
    }
}
```

И вызовите в `CheckProtection()`:
```cpp
void CheckProtection() {
    // ...
    if (CheckCustom()) {
        _exit(0);
    }
}
```

---

## Troubleshooting

### Проблема: "Build failed"
```bash
# Проверьте NDK
echo $ANDROID_NDK_HOME
ls $ANDROID_NDK_HOME/ndk-build

# Проверьте SDK
echo $ANDROID_HOME
ls $ANDROID_HOME/build-tools/*/d8
```

### Проблема: "APK signing failed"
```bash
# Переустановите Java keytool
sudo apt-get install --reinstall default-jdk

# Или используйте существующий keystore
./auto_build_and_inject.sh /path/to/apk --keystore /path/to/your.keystore
```

### Проблема: "Overlay permission denied"
```bash
# Дайте разрешение вручную
adb shell appops set com.axlebolt.standoff2 SYSTEM_ALERT_WINDOW allow

# Или через настройки:
# Settings → Apps → Standoff 2 → Display over other apps → Allow
```

### Проблема: "Mod not loading"
```bash
# Проверьте что DEX загружен
adb logcat | grep DexInjector

# Должно быть:
# DexInjector: DEX injected successfully!
```

---

## Безопасность использования

### ✅ Рекомендации:
- Используйте на тестовых аккаунтах
- Не включайте стрим во время использования
- Держите умеренные настройки (FOV 60-90°, Smooth 5-10)
- Не используйте в ranked режимах

### ❌ Не делайте:
- Не используйте на основном аккаунте
- Не ставьте экстремальные значения
- Не стримьте с активным модом
- Не продавайте модифицированный APK

---

## Итоговый чеклист

- [ ] Установлены NDK и SDK
- [ ] Переменные окружения настроены
- [ ] Запущен `./auto_build_and_inject.sh standoff2.apk`
- [ ] Получен `standoff2_modded_signed.apk`
- [ ] APK установлен: `adb install -r standoff2_modded_signed.apk`
- [ ] Дано разрешение overlay
- [ ] Игра запущена
- [ ] Кнопка "Menu" видна в игре
- [ ] Меню открывается
- [ ] ESP/Aimbot работают

**Если все ✅ - поздравляем, мод установлен!** 🎉

---

## Контакты и поддержка

- **Логи**: `adb logcat | grep ModMenu`
- **Документация**: См. другие MD файлы в проекте
- **Issues**: Создавайте issue с логами

**Версия**: 2.0.0-rootless-protected  
**Дата**: 2024-02-15  
**Статус**: ✅ Production Ready
