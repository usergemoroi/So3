# 🎮 Standoff 2 - Rootless Mod Menu System

## 🌟 Что это?

Это **полностью безрутовая** система модификации для Standoff 2, позволяющая использовать ESP, Aimbot и другие функции **БЕЗ ROOT ПРАВ**.

```
┌────────────────────────────────────────────────┐
│  ✅ БЕЗ ROOT      ✅ БЕЗ XPOSED      ✅ РАБОТАЕТ │
│  ✅ Android 7+    ✅ ARM64/ARM32     ✅ Защищено│
└────────────────────────────────────────────────┘
```

## 📦 Структура проекта

```
.
├── mod_menu_rootless/              ← НОВАЯ БЕЗРУТОВАЯ СИСТЕМА
│   ├── java/                       # Android Java код
│   │   └── com/modmenu/loader/
│   │       ├── ModMenuLoader.java      # Главный загрузчик
│   │       ├── OverlayService.java     # Overlay UI меню
│   │       ├── DexInjector.java        # DEX инъекция
│   │       └── LifecycleListener.java  # Activity hooks
│   │
│   ├── jni/                        # Native C++ код
│   │   ├── main_noroot.cpp             # JNI entry point
│   │   ├── il2cpp_noroot.hpp           # IL2CPP resolver
│   │   ├── hooks_noroot.hpp            # Function hooking
│   │   ├── protection_noroot.hpp       # Anti-debug/emulator
│   │   ├── esp_noroot.hpp              # ESP визуализация
│   │   ├── aimbot_noroot.hpp           # Aimbot логика
│   │   └── substrate.h                 # Hooking framework
│   │
│   ├── tools/                      # Утилиты
│   │   ├── apk_patcher.py             # Автоматический патчер APK
│   │   ├── install.sh                 # Быстрая установка
│   │   ├── test_device.sh             # Проверка совместимости
│   │   ├── debug_logs.sh              # Отладочные логи
│   │   └── cleanup.sh                 # Очистка
│   │
│   ├── build.sh                    # Скрипт сборки
│   ├── README.md                   # Главная документация
│   ├── INTEGRATION_GUIDE.md        # Руководство по интеграции
│   ├── FEATURES.md                 # Полный список функций
│   └── CHANGELOG.md                # История версий
│
├── mod_menu/                       ← Старая версия (с root)
│   └── ...
│
├── QUICK_START.md                  # Быстрый старт за 5 минут
├── ROOTLESS_SUMMARY.md             # Техническая документация
└── README_ROOTLESS.md              # Этот файл
```

## 🚀 Быстрый старт

### За 5 минут от нуля до готового APK

```bash
# 1. Установите зависимости (один раз)
sudo apt-get install openjdk-11-jdk python3 git wget unzip

# Скачайте Android NDK
wget https://dl.google.com/android/repository/android-ndk-r25c-linux.zip
unzip android-ndk-r25c-linux.zip
export ANDROID_NDK_HOME=$PWD/android-ndk-r25c

# Скачайте Android SDK
wget https://dl.google.com/android/repository/commandlinetools-linux-9477386_latest.zip
mkdir -p android-sdk/cmdline-tools
unzip commandlinetools-linux-9477386_latest.zip -d android-sdk/cmdline-tools
mv android-sdk/cmdline-tools/cmdline-tools android-sdk/cmdline-tools/latest
export ANDROID_HOME=$PWD/android-sdk
$ANDROID_HOME/cmdline-tools/latest/bin/sdkmanager --sdk_root=$ANDROID_HOME \
    "build-tools;33.0.0" "platforms;android-33"

# Установите apktool
wget https://raw.githubusercontent.com/iBotPeaches/Apktool/master/scripts/linux/apktool
wget https://bitbucket.org/iBotPeaches/apktool/downloads/apktool_2.8.1.jar
chmod +x apktool
sudo mv apktool apktool_2.8.1.jar /usr/local/bin/

# 2. Соберите мод
cd mod_menu_rootless
./build.sh

# 3. Запатчите APK
cd tools
python3 apk_patcher.py /path/to/standoff2.apk

# 4. Установите
adb install standoff2_modded_signed.apk
```

**Готово!** 🎉 Откройте игру и найдите кнопку "Menu" в левом верхнем углу.

## 📖 Полная документация

### Для пользователей

- **[QUICK_START.md](QUICK_START.md)** - Быстрый старт за 5 минут
- **[mod_menu_rootless/README.md](mod_menu_rootless/README.md)** - Основная документация
- **[mod_menu_rootless/INTEGRATION_GUIDE.md](mod_menu_rootless/INTEGRATION_GUIDE.md)** - Детальное руководство
- **[mod_menu_rootless/FEATURES.md](mod_menu_rootless/FEATURES.md)** - Все функции

### Для разработчиков

- **[ROOTLESS_SUMMARY.md](ROOTLESS_SUMMARY.md)** - Техническая документация
- **[mod_menu_rootless/CHANGELOG.md](mod_menu_rootless/CHANGELOG.md)** - История изменений

## ✨ Основные функции

### 🎯 ESP (Wallhack)
- **Box ESP** - Рамки вокруг игроков
- **Skeleton ESP** - Отображение скелета
- **Health ESP** - Показ здоровья
- **Distance ESP** - Расстояние до игрока
- **Name ESP** - Имя игрока

### 🎯 Aimbot
- **Smart Targeting** - Умный выбор цели
- **FOV Control** - Настраиваемый угол обзора (10-180°)
- **Smooth Aim** - Плавное наведение (1.0-20.0)
- **Visible Only** - Цель только по видимым
- **Team Check** - Проверка команды

### 🛡️ Защита
- **Anti-Debug** - Защита от отладчиков
- **Anti-Emulator** - Детект эмуляторов
- **Memory Integrity** - Проверка целостности памяти
- **String Encryption** - Шифрование строк

### 🎨 UI
- **Android Overlay** - Меню поверх игры
- **Draggable Button** - Перетаскиваемая кнопка
- **Real-time Config** - Настройка в реальном времени
- **No OpenGL** - Не модифицирует игровой рендер

## 🎮 Как использовать

### 1. Первый запуск

После установки модифицированного APK:

```
1. Откройте Standoff 2
2. Разрешите "Отображение поверх других окон"
   (появится запрос при первом запуске)
3. Найдите кнопку "Menu" в левом верхнем углу
4. Нажмите для открытия меню
5. Настройте функции
6. Играйте!
```

### 2. Настройка ESP

```
┌─────────────────────────────┐
│      ESP Settings           │
├─────────────────────────────┤
│ ☑ Enable ESP               │
│ ☑ Box ESP                  │
│ ☑ Skeleton ESP             │
│ ☑ Health Bar               │
│ ☐ Distance                 │
│ ☐ Name                     │
└─────────────────────────────┘
```

### 3. Настройка Aimbot

```
┌─────────────────────────────┐
│     Aimbot Settings         │
├─────────────────────────────┤
│ ☑ Enable Aimbot            │
│ ☑ Visible Only             │
│ ☑ Team Check               │
│                            │
│ FOV: [====●=====] 90°      │
│ Smooth: [==●========] 5.0  │
└─────────────────────────────┘
```

## 🔧 Решение проблем

### Меню не появляется?

```bash
# Проверьте разрешение overlay
adb shell appops get com.axlebolt.standoff2 SYSTEM_ALERT_WINDOW

# Дайте разрешение вручную
adb shell appops set com.axlebolt.standoff2 SYSTEM_ALERT_WINDOW allow

# Посмотрите логи
adb logcat | grep ModMenu
```

### Игра крашится?

```bash
# Посмотрите логи краша
adb logcat | grep "FATAL\|AndroidRuntime"

# Проверьте совместимость устройства
cd mod_menu_rootless/tools
./test_device.sh
```

### ESP не работает?

```bash
# Проверьте инициализацию IL2CPP
adb logcat | grep IL2CPP

# Проверьте установку хуков
adb logcat | grep Hooks
```

## 📊 Сравнение версий

| Функция | Старая (root) | Новая (rootless) |
|---------|--------------|------------------|
| **Root требуется** | ✅ Да | ❌ Нет |
| **Xposed нужен** | ✅ Да | ❌ Нет |
| **UI тип** | ImGui (OpenGL) | Android Overlay |
| **Загрузка** | LD_PRELOAD | DEX Injection |
| **Защита** | Базовая | Продвинутая |
| **Совместимость** | Android 5+ | Android 7+ |
| **Обнаружение** | Высокий риск | Низкий риск |

## 🛡️ Безопасность

### ✅ Что БЕЗОПАСНО:

- Использование в одиночных режимах
- Тестирование на новых аккаунтах
- Умеренные настройки (FOV 60-90°, Smooth 5-10)
- Использование на реальных устройствах

### ❌ Что ОПАСНО:

- Ranked режимы
- Основной аккаунт
- Экстремальные настройки (FOV 180°, Smooth 1.0)
- Стримы с модом
- Использование на эмуляторах

## 📱 Требования

### Минимальные
- Android 7.0+ (API 24)
- ARM64 или ARM32 процессор
- 2 GB RAM
- 100 MB свободного места

### Рекомендуемые
- Android 10.0+ (API 29)
- ARM64 процессор
- 4 GB RAM
- Реальное устройство (не эмулятор)

## 🔄 Обновление

После обновления игры:

```bash
# 1. Обновите репозиторий
git pull

# 2. Пересоберите мод
cd mod_menu_rootless
./build.sh

# 3. Перепатчите новый APK
cd tools
python3 apk_patcher.py /path/to/new_standoff2.apk

# 4. Переустановите
adb install -r standoff2_modded_signed.apk
```

## 🤝 Поддержка

### Документация
- 📘 Читайте [QUICK_START.md](QUICK_START.md) для быстрого старта
- 📖 Смотрите [INTEGRATION_GUIDE.md](mod_menu_rootless/INTEGRATION_GUIDE.md) для деталей
- 🔧 Изучите [ROOTLESS_SUMMARY.md](ROOTLESS_SUMMARY.md) для технических деталей

### Утилиты отладки
```bash
cd mod_menu_rootless/tools

# Проверка совместимости
./test_device.sh

# Логи в реальном времени
./debug_logs.sh

# Очистка временных файлов
./cleanup.sh
```

### Связь
- **Issues**: GitHub Issues
- **Discussions**: GitHub Discussions
- **Email**: support@example.com

## 📈 Статистика

- **Время сборки**: ~2-3 минуты
- **Время патчинга**: ~1-2 минуты
- **Размер мода**: ~5 MB (DEX + SO)
- **Влияние на FPS**: -0 до -2 FPS
- **Использование RAM**: +20-30 MB

## ⚖️ Правовая информация

### ⚠️ Disclaimer

Этот проект создан **исключительно в образовательных целях** для изучения:
- Android reverse engineering
- DEX injection техник
- Native hooking на ARM
- Unity IL2CPP структуры
- Anti-cheat обход механизмов

**ВАЖНО:**
- Использование модов в онлайн играх нарушает ToS
- Может привести к перманентному бану
- Портит опыт другим игрокам
- Авторы не несут ответственности за использование

### License

MIT License - см. [LICENSE](LICENSE)

## 🎖️ Credits

**Технологии:**
- [Android NDK](https://developer.android.com/ndk)
- [Cydia Substrate](https://github.com/Cydia/substrate)
- [apktool](https://ibotpeaches.github.io/Apktool/)
- [Il2CppDumper](https://github.com/Perfare/Il2CppDumper)

**Спасибо:**
- Всем контрибьюторам
- Тестерам
- Сообществу Android modding

## 📊 Roadmap

### v2.1.0 (Планируется)
- [ ] Radar hack
- [ ] No recoil
- [ ] Speed hack
- [ ] Customizable themes
- [ ] Hotkey system

### v3.0.0 (Будущее)
- [ ] Multi-game support
- [ ] Plugin system
- [ ] Cloud configs
- [ ] Web dashboard
- [ ] Auto-updates

## 🏆 Заключение

Полнофункциональная безрутовая система модификации готова к использованию!

**Ключевые преимущества:**
- ✅ Не требует root
- ✅ Простая установка
- ✅ Защита от обнаружения
- ✅ Полная документация
- ✅ Утилиты для отладки

**Начните сейчас:**
```bash
cd mod_menu_rootless
./build.sh
cd tools
python3 apk_patcher.py /path/to/standoff2.apk
adb install standoff2_modded_signed.apk
```

---

**Версия:** 2.0.0-rootless  
**Статус:** ✅ Работает  
**Root:** ❌ Не требуется  
**Дата:** 2024-02-15

**Удачи!** 🎮🚀
