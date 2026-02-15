# Руководство по установке и сборке

## Текущий статус

✅ Java Compiler (javac) - установлен (версия 17.0.18)
❌ Android NDK - не установлен
❌ Android SDK - не установлен

## Вариант 1: Установка Android NDK и SDK

### Установка Android SDK Command Line Tools

```bash
# Скачайте Android Command Line Tools для Linux
# https://developer.android.com/studio#command-tools

mkdir -p ~/android-sdk
cd ~/android-sdk
wget https://dl.google.com/android/repository/commandlinetools-linux-9477386_latest.zip
unzip commandlinetools-linux-9477386_latest.zip
mkdir -p cmdline-tools/latest
mv cmdline-tools/* cmdline-tools/latest/
```

```bash
export ANDROID_HOME=~/android-sdk
export PATH=$PATH:$ANDROID_HOME/cmdline-tools/latest/bin:$ANDROID_HOME/platform-tools

# Установите необходимые пакеты
sdkmanager "platform-tools" "platforms;android-30" "build-tools;30.0.3"
```

### Установка Android NDK

```bash
# Через sdkmanager
sdkmanager "ndk;25.2.9519653"

export ANDROID_NDK_HOME=$ANDROID_HOME/ndk/25.2.9519653
```

### Добавление в ~/.bashrc

```bash
echo 'export ANDROID_HOME=~/android-sdk' >> ~/.bashrc
echo 'export ANDROID_NDK_HOME=~/android-sdk/ndk/25.2.9519653' >> ~/.bashrc
echo 'export PATH=$PATH:$ANDROID_HOME/cmdline-tools/latest/bin:$ANDROID_HOME/platform-tools' >> ~/.bashrc
source ~/.bashrc
```

## Вариант 2: Использование Docker (рекомендуется)

Создайте Dockerfile:

```dockerfile
FROM ubuntu:22.04

# Установка базовых зависимостей
RUN apt-get update && apt-get install -y \
    wget \
    unzip \
    openjdk-17-jdk \
    build-essential \
    cmake \
    git \
    && rm -rf /var/lib/apt/lists/*

# Установка Android SDK
ENV ANDROID_HOME=/opt/android-sdk
ENV ANDROID_NDK_HOME=/opt/android-sdk/ndk/25.2.9519653
ENV PATH=$PATH:$ANDROID_HOME/cmdline-tools/latest/bin:$ANDROID_HOME/platform-tools

RUN mkdir -p $ANDROID_HOME/cmdline-tools && \
    wget https://dl.google.com/android/repository/commandlinetools-linux-9477386_latest.zip && \
    unzip commandlinetools-linux-9477386_latest.zip -d $ANDROID_HOME/cmdline-tools && \
    mv $ANDROID_HOME/cmdline-tools/cmdline-tools $ANDROID_HOME/cmdline-tools/latest && \
    rm commandlinetools-linux-9477386_latest.zip

# Установка SDK компонентов
RUN yes | sdkmanager --licenses && \
    sdkmanager "platform-tools" "platforms;android-30" "build-tools;30.0.3" "ndk;25.2.9519653"

WORKDIR /workspace
```

Сборка и запуск:

```bash
docker build -t android-build .
docker run -it -v $(pwd):/workspace android-build bash

# Внутри контейнера:
cd /workspace/mod_menu_rootless
./build_and_inject_all.sh
```

## Вариант 3: Компиляция только Java части (без native)

Если вам нужно только скомпилировать Java классы в DEX:

```bash
cd /home/engine/project/mod_menu_rootless

mkdir -p build/classes

# Компиляция Java классов
javac -d build/classes \
    -source 1.8 -target 1.8 \
    java/com/modmenu/loader/*.java

# Для создания DEX нужен d8 из Android SDK build-tools
# Без Android SDK это невозможно
```

## Минимальные требования для сборки

### Для сборки libvis.so:
- Android NDK 21+ (рекомендуется 25.x)
- GCC/Clang компилятор (входит в NDK)
- cmake или ndk-build

### Для сборки classes.dex:
- Java JDK 8+ ✅ (уже установлен)
- Android SDK build-tools (d8 инструмент)
- android.jar для bootclasspath

## Проверка установки

После установки проверьте:

```bash
# Проверка NDK
$ANDROID_NDK_HOME/ndk-build --version

# Проверка SDK
sdkmanager --list

# Проверка d8
ls $ANDROID_HOME/build-tools/*/d8

# Проверка javac
javac -version
```

## Быстрый тест после установки

```bash
export ANDROID_HOME=~/android-sdk
export ANDROID_NDK_HOME=~/android-sdk/ndk/25.2.9519653

cd /home/engine/project/mod_menu_rootless
./build_and_inject_all.sh
```

## Ссылки для загрузки

- Android Command Line Tools: https://developer.android.com/studio#command-tools
- Android NDK: https://developer.android.com/ndk/downloads
- Android Studio (включает SDK и NDK): https://developer.android.com/studio

## Альтернатива: Использование уже готовых бинарных файлов

Если у вас нет желания устанавливать NDK/SDK, вы можете:

1. Скомпилировать на другом компьютере
2. Использовать CI/CD (GitHub Actions, GitLab CI)
3. Скачать готовые файлы (если они доступны)

Пример GitHub Actions workflow:

```yaml
name: Build Mod Menu

on: [push]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3

      - name: Set up JDK 17
        uses: actions/setup-java@v3
        with:
          java-version: '17'
          distribution: 'temurin'

      - name: Set up Android SDK
        uses: android-actions/setup-android@v2

      - name: Build native library
        run: |
          cd mod_menu_rootless/jni
          $ANDROID_NDK_HOME/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk

      - name: Build Java classes
        run: |
          cd mod_menu_rootless
          mkdir -p build/classes
          javac -d build/classes -source 1.8 -target 1.8 \
            -bootclasspath $ANDROID_HOME/platforms/android-30/android.jar \
            java/com/modmenu/loader/*.java
          $ANDROID_HOME/build-tools/*/d8 \
            --lib $ANDROID_HOME/platforms/android-30/android.jar \
            --release \
            --output build/ \
            build/classes/com/modmenu/loader/*.class

      - name: Upload artifacts
        uses: actions/upload-artifact@v3
        with:
          name: mod-menu-files
          path: |
            mod_menu_rootless/libs/**/*.so
            mod_menu_rootless/build/classes.dex
```
