# Технические детали Standoff 2 Mod Menu

## Архитектура проекта

### Структура модуля

```
mod_menu/
├── jni/                          # Нативный C++ код
│   ├── main.cpp                  # Точка входа, инициализация, хуки
│   ├── il2cpp_resolver.hpp       # Работа с Unity IL2CPP
│   ├── esp.h                     # Реализация ESP функций
│   ├── aimbot.h                  # Реализация аимбота
│   ├── imgui/                    # GUI библиотека
│   │   ├── imgui.h
│   │   ├── imgui.cpp
│   │   ├── imgui_impl_opengl3.h
│   │   └── imgui_impl_android.h
│   ├── Android.mk                # NDK build скрипт
│   └── Application.mk            # NDK конфигурация
├── build.sh                      # Скрипт автоматической сборки
├── README.md                     # Основная документация
├── USAGE.md                      # Руководство пользователя
└── TECHNICAL.md                  # Этот файл
```

## Анализ дампа

### Структура PlayerController

Из дампа `dump (3).cs` (строка 247708):

```cpp
public class PlayerController : PunBehaviour, FGBHGHGDGHBGDAF // TypeDefIndex: 7858
{
    // Offset 0x28
    private Transform _mainCameraHolder;        // 0x28
    private GameObject _fpsCameraHolder;        // 0x30
    private GameObject _fpsDirective;           // 0x38
    private PlayerLevelZonesController AAHHFDECFDHBHHG; // 0x40
    private PlayerCharacterView DDFAGDFBHDECGBC;        // 0x48
    private PlayerCharacterView ADAEHECFGAFHCCD;        // 0x50
    // ... другие поля
    private Transform CDEFDCAHEGAHBGD;          // 0xF8 - Transform игрока
    // ...
}
```

### Важные структуры

**Vector3** (Unity стандарт):
```cpp
struct Vector3 {
    float x;  // +0x00
    float y;  // +0x04
    float z;  // +0x08
};
```

**Transform** (Unity компонент):
```cpp
struct Transform {
    void** vtable;        // +0x00
    void* m_CachedPtr;    // +0x08
    // ... другие поля
    Vector3 position;     // Получается через методы
};
```

**BipedMap** (карта костей персонажа):
```cpp
struct BipedMap {
    Transform* head;          // 0 - Голова
    Transform* neck;          // 1 - Шея
    Transform* chest;         // 2 - Грудь
    Transform* spine;         // 3 - Позвоночник
    Transform* hips;          // 4 - Бедра
    Transform* leftShoulder;  // 5 - Левое плечо
    Transform* leftUpperArm;  // 6 - Левое предплечье
    Transform* leftForearm;   // 7 - Левая рука
    Transform* leftHand;      // 8 - Левая кисть
    Transform* rightShoulder; // 9 - Правое плечо
    Transform* rightUpperArm; // 10 - Правое предплечье
    Transform* rightForearm;  // 11 - Правая рука
    Transform* rightHand;     // 12 - Правая кисть
    Transform* leftThigh;     // 13 - Левое бедро
    Transform* leftCalf;      // 14 - Левая голень
    Transform* leftFoot;      // 15 - Левая стопа
    Transform* rightThigh;    // 16 - Правое бедро
    Transform* rightCalf;     // 17 - Правая голень
    Transform* rightFoot;     // 18 - Правая стопа
};
```

## Реализация функций

### ESP (Extra Sensory Perception)

#### Скелеты (Skeleton)

Алгоритм рисования скелета:
1. Получить указатель на PlayerController
2. Получить BipedMap (карту костей)
3. Для каждой кости получить Transform
4. Преобразовать мировые координаты в экранные (WorldToScreen)
5. Нарисовать линии между соединенными костями

Соединения костей (19 точек):
```cpp
const int boneConnections[][2] = {
    {0, 1},   // Голова -> Шея
    {1, 2},   // Шея -> Грудь
    {2, 3},   // Грудь -> Позвоночник
    {3, 4},   // Позвоночник -> Бедра
    {1, 5},   // Шея -> Левое плечо
    {5, 6},   // Левое плечо -> Левое предплечье
    {6, 7},   // Левое предплечье -> Левая рука
    {7, 8},   // Левая рука -> Левая кисть
    {1, 9},   // Шея -> Правое плечо
    {9, 10},  // Правое плечо -> Правое предплечье
    {10, 11}, // Правое предплечье -> Правая рука
    {11, 12}, // Правая рука -> Правая кисть
    {4, 13},  // Бедра -> Левое бедро
    {13, 14}, // Левое бедро -> Левая голень
    {14, 15}, // Левая голень -> Левая стопа
    {4, 16},  // Бедра -> Правое бедро
    {16, 17}, // Правое бедро -> Правая голень
    {17, 18}  // Правая голень -> Правая стопа
};
```

#### WorldToScreen

Преобразование 3D координат в 2D экранные:
```cpp
bool WorldToScreen(Vector3 world, Vector2& screen) {
    Matrix4x4 viewMatrix = GetViewMatrix();
    Matrix4x4 projMatrix = GetProjectionMatrix();
    
    Vector4 clipCoords = projMatrix * viewMatrix * Vector4(world, 1.0f);
    
    if (clipCoords.w < 0.1f) return false;
    
    Vector3 ndc;
    ndc.x = clipCoords.x / clipCoords.w;
    ndc.y = clipCoords.y / clipCoords.w;
    ndc.z = clipCoords.z / clipCoords.w;
    
    screen.x = (ndc.x + 1.0f) * (screenWidth / 2.0f);
    screen.y = (1.0f - ndc.y) * (screenHeight / 2.0f);
    
    return true;
}
```

### Aimbot

#### Алгоритм работы:

1. **Получение локального игрока**
   ```cpp
   PlayerController* localPlayer = GetLocalPlayer();
   ```

2. **Поиск целей**
   ```cpp
   std::vector<PlayerController*> players = GetAllPlayers();
   for (auto player : players) {
       if (player == localPlayer) continue;
       if (IsTeammate(player, localPlayer)) continue;
       // Добавить в список целей
   }
   ```

3. **Выбор лучшей цели**
   - Вычислить FOV для каждой цели
   - Выбрать цель с минимальным FOV
   - Проверить видимость (если включено)
   
4. **Наведение на цель**
   ```cpp
   Vector3 targetPos = GetHeadPosition(target);
   Vector3 delta = targetPos - cameraPos;
   
   float yaw = atan2(delta.x, delta.z);
   float pitch = atan2(delta.y, sqrt(delta.x*delta.x + delta.z*delta.z));
   
   // Применить плавность
   float smoothFactor = 1.0f / config.smooth;
   currentYaw += (yaw - currentYaw) * smoothFactor;
   currentPitch += (pitch - currentPitch) * smoothFactor;
   
   SetCameraRotation(currentYaw, currentPitch);
   ```

## Хуки и патчи

### eglSwapBuffers Hook

Перехват функции рендеринга для рисования GUI и ESP:

```cpp
EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay, EGLSurface);

EGLBoolean hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    // Инициализация ImGui (один раз)
    if (!g_ImGuiInitialized) {
        InitImGui();
        g_ImGuiInitialized = true;
    }
    
    // Новый кадр
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame();
    ImGui::NewFrame();
    
    // Рисование GUI
    RenderMenu();
    
    // Рисование ESP
    if (g_Config.esp_enabled) {
        ESP::Draw(g_Config);
    }
    
    // Рендер
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    // Вызов оригинальной функции
    return orig_eglSwapBuffers(dpy, surface);
}
```

### IL2CPP Resolver

Получение функций Unity через IL2CPP:

```cpp
void* il2cpp_handle = dlopen("libil2cpp.so", RTLD_NOW);

// Получение функций
il2cpp_domain_get = (il2cpp_domain_get_t)
    dlsym(il2cpp_handle, "il2cpp_domain_get");

il2cpp_class_from_name = (il2cpp_class_from_name_t)
    dlsym(il2cpp_handle, "il2cpp_class_from_name");

// Поиск класса PlayerController
void* domain = il2cpp_domain_get();
void* image = GetImageByName("Assembly-CSharp");
void* klass = il2cpp_class_from_name(image, "", "PlayerController");
```

## Оптимизация

### Производительность

**Кэширование:**
- Кэшировать список игроков (обновлять каждые 100мс)
- Кэшировать матрицы view/projection
- Использовать object pooling для временных объектов

**Многопоточность:**
- ESP расчеты в отдельном потоке
- Синхронизация через mutex
- Рендеринг в главном потоке

**Оптимизация рендеринга:**
- Батчинг draw calls
- Использование VBO для статической геометрии
- Culling невидимых объектов

## Обход античита

### Методы обнаружения

1. **Memory scanning** - сканирование памяти
2. **Integrity checks** - проверка целостности
3. **Hook detection** - обнаружение хуков
4. **Behavioral analysis** - анализ поведения

### Методы обхода

1. **Memory protection**
   ```cpp
   mprotect(page, size, PROT_READ | PROT_WRITE | PROT_EXEC);
   ```

2. **Code obfuscation**
   - Переименование символов
   - Шифрование строк
   - Виртуализация кода

3. **Anti-detection**
   - Случайные задержки
   - Имитация человеческого поведения
   - Изменение паттернов

## Отладка

### LogCat фильтры

```bash
# Все логи модуля
adb logcat | grep ModMenu

# Ошибки IL2CPP
adb logcat | grep IL2CPP

# OpenGL ошибки
adb logcat | grep GLES

# Крэши
adb logcat | grep "FATAL EXCEPTION"
```

### GDB Debugging

```bash
# Подключение к процессу
adb shell gdbserver :5039 --attach $(pidof com.axlebolt.standoff2)

# На PC
adb forward tcp:5039 tcp:5039
gdb-multiarch
target remote :5039
```

### Memory Dumps

```bash
# Дамп памяти процесса
adb shell "su -c 'cat /proc/$(pidof com.axlebolt.standoff2)/maps'"

# Дамп конкретного региона
adb shell "su -c 'dd if=/proc/$(pidof com.axlebolt.standoff2)/mem \
  of=/sdcard/dump.bin skip=$START count=$SIZE bs=1'"
```

## Расширения

### Добавление новых функций

1. **Speed hack**
   - Найти функцию движения
   - Патчить множитель скорости
   - Добавить ползунок в GUI

2. **No recoil**
   - Найти функцию отдачи
   - Обнулить или уменьшить отдачу
   - Добавить переключатель в GUI

3. **Radar hack**
   - Получить позиции всех игроков
   - Нарисовать мини-карту
   - Отобразить позиции точками

### Пример добавления функции

```cpp
// В ModConfig
bool no_recoil = false;

// В GUI
if (ImGui::CollapsingHeader("Weapon Settings")) {
    ImGui::Checkbox("No Recoil", &g_Config.no_recoil);
}

// В хуке Update
if (g_Config.no_recoil) {
    ApplyNoRecoil();
}
```

## Безопасность кода

### Best practices

1. **Никогда не доверяйте входным данным**
2. **Проверяйте все указатели на nullptr**
3. **Используйте bounds checking**
4. **Избегайте buffer overflow**
5. **Освобождайте память правильно**

### Пример безопасного кода

```cpp
// ❌ Небезопасно
Vector3 GetPosition(Transform* transform) {
    return *(Vector3*)(transform + 0x10);
}

// ✅ Безопасно
Vector3 GetPosition(Transform* transform) {
    if (!transform) return Vector3();
    if (IsBadReadPtr(transform, sizeof(Transform))) return Vector3();
    return *(Vector3*)(transform + 0x10);
}
```

## Лицензия и ответственность

Этот проект создан исключительно для образовательных целей.
Использование в реальной игре нарушает условия использования.
Автор не несет ответственности за последствия использования.
