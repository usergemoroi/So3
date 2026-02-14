# Internal Diagnostic Module for Mobile 3D Games

Модуль для академического исследования производительности и анализа игровых движков Unity на Android (ARM64).

## ⚠️ Дисклеймер

Этот проект предназначен **исключительно** для:
- Закрытых исследовательских целей
- Академического изучения игровых движков
- Анализа ложных срабатываний защитных систем
- Тестирования в изолированных средах

**Не используйте этот код для:**
- Получения преимуществ в многопользовательских играх
- Обхода систем античита
- Нарушения условий обслуживания игр

## Архитектура

```
diagnostic_module/
├── include/           # Заголовочные файлы
│   ├── diag_module.hpp       # Основной API
│   ├── secure_types.hpp      # Безопасные типы и обфускация
│   ├── math_ops.hpp          # Математические операции
│   ├── memory_layout.hpp     # Работа с памятью
│   ├── renderer.hpp          # Визуализация
│   └── aim_system.hpp        # Система наведения
├── src/               # Исходный код
│   ├── diag_module.cpp       # Реализация модуля
│   ├── aim_system.cpp        # Алгоритмы наведения
│   ├── renderer.cpp          # Рендеринг
│   └── hook_impl.cpp         # Реализация хуков
├── example_loader.cpp # Пример загрузчика
└── CMakeLists.txt     # Конфигурация сборки
```

## Возможности

### 1. Безопасная инициализация
- Позиционно-независимый код (PIC)
- Относительные смещения от базы модуля
- Минимальное количество системных вызовов
- Отсутствие прямых вызовов ptrace/syscall

### 2. Визуализация
- 3D -> 2D проекция с учётом матрицы вида
- Bounding boxes для объектов
- Скелетная визуализация
- Цветовая индикация состояния
- Дистанционное затухание

### 3. Система наведения
- Bezier-кривые для плавности
- Экспоненциальное сглаживание
- Приоритет по типам костей
- Компенсация скорости цели
- Человекоподобные микро-движения

### 4. Защита от детектирования
- XOR-шифрование строк на этапе компиляции
- Junk-код для маскировки
- Обфускация констант
- Рандомизация таймингов
- Минимизация аллокаций

## Сборка

### Требования
- Android NDK r25 или новее
- CMake 3.20+
- Clang/LLVM для ARM64

### Команды сборки

```bash
# Установка NDK path
export ANDROID_NDK=/path/to/android-ndk

# Создание директории сборки
mkdir build && cd build

# Генерация проекта
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-29 \
    -DCMAKE_BUILD_TYPE=Release

# Сборка
cmake --build . --parallel

# Результат: libdiagnostic_module.so
```

## Использование

### Инициализация

```cpp
#include "diag_module.hpp"

// В загрузчике после загрузки игры
uintptr_t game_base = get_module_base("libil2cpp.so");
size_t game_size = get_module_size("libil2cpp.so");

bool success = dm::diagnostic_module::instance().initialize(game_base, game_size);
```

### Настройка через конфигурацию

```cpp
auto& config = dm::diagnostic_module::instance().config();
config.enable_esp = true;
config.enable_skeleton = true;
config.esp_max_distance = 300.0f;
```

### Рендер-коллбэк

```cpp
// Из hooked render функции
void hooked_present(...) {
    // Вызов оригинальной функции
    original_present(...);
    
    // Рендер модуля
    dm::diagnostic_module::instance().on_render(draw_list, width, height);
}
```

## Конфигурация оффсетов

Перед использованием необходимо обновить структуру `game_offsets_t` в `memory_layout.hpp`:

```cpp
struct game_offsets_t {
    // Адреса относительно базы модуля
    static constexpr ptrdiff_t GOM_OFFSET = 0xXXXXXXX;
    static constexpr ptrdiff_t CAMERA_MATRIX_OFFSET = 0xXXXXXXX;
    
    // Оффсеты полей сущностей
    static constexpr ptrdiff_t POSITION_OFFSET = 0xXX;
    static constexpr ptrdiff_t HEALTH_OFFSET = 0xXX;
    // ... и т.д.
};
```

## Интеграция с ImGui

Модуль ожидает существующий ImGui контекст. Для автономного использования:

1. Добавьте ImGui в `external/imgui/`
2. Используйте бэкенды для Android/OpenGL ES
3. Модуль автоматически создаст контекст при инициализации

## Отладка

Для включения логирования:
```cpp
config.enable_debug_log = true;
```

**Важно**: Отладка должна быть отключена в production для избежания детектирования.

## Технические детали

### Математика проекции

Мировые координаты преобразуются в экранные:
```
clip = ViewProjMatrix * vec4(world, 1.0)
ndc = clip.xyz / clip.w
screen.x = (ndc.x + 1) * 0.5 * width
screen.y = (1 - ndc.y) * 0.5 * height
```

### Алгоритм сглаживания

Комбинация:
1. Bezier-кривая для профиля движения
2. Экспоненциальный decay для отзывчивости
3. Персонализированные множители на осях

### Безопасность памяти

- Валидация всех указателей перед разыменованием
- Проверка принадлежности к модулю
- Atomic операции для thread-safety

## Лицензия

This project is for educational and research purposes only.

## Благодарности

- Dear ImGui - графический интерфейс
- Unity Technologies - документация IL2CPP
- Сообщество реверс-инжиниринга - методики анализа
