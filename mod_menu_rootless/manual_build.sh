#!/bin/bash
# Скрипт ручной сборки компонентов mod_menu_rootless
# Этот скрипт создает необходимые файлы без использования NDK

set -e

echo "=========================================="
echo "  Manual Build Script for mod_menu_rootless"
echo "=========================================="
echo ""

# Создаем структуру папок
mkdir -p libs/arm64-v8a
mkdir -p libs/armeabi-v7a
mkdir -p build/classes/com/modmenu/loader

echo "[1/4] Creating stub native library..."

# Создаем простой ELF заглушку для ARM64
cat > /tmp/create_stub_so.sh << 'EOF'
#!/bin/bash
ARCH=$1
OUTPUT=$2

# Простейшая 64-битная ARM ELF библиотека-заглушка
if [ "$ARCH" = "arm64" ]; then
cat > "$OUTPUT" << 'ELFEOT'
#define ELFMAGIC "\x7fELF"
#define ELFCLASS64 2
#define ELFDATA2LSB 1
#define EM_AARCH64 183

typedef struct {
    unsigned char e_ident[4];
    unsigned char e_class;
    unsigned char e_data;
    unsigned char e_version;
    unsigned char e_osabi;
    unsigned char e_abiversion;
    unsigned char e_pad[7];
    unsigned short e_type;
    unsigned short e_machine;
    unsigned int e_version2;
    unsigned long e_entry;
    unsigned long e_phoff;
    unsigned long e_shoff;
    unsigned int e_flags;
    unsigned short e_ehsize;
    unsigned short e_phentsize;
    unsigned short e_phnum;
    unsigned short e_shentsize;
    unsigned short e_shnum;
    unsigned short e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    unsigned int p_type;
    unsigned int p_flags;
    unsigned long p_offset;
    unsigned long p_vaddr;
    unsigned long p_paddr;
    unsigned long p_filesz;
    unsigned long p_memsz;
    unsigned long p_align;
} Elf64_Phdr;

int main() {
    // Stub library
    return 0;
}
ELFEOT
fi
EOF

chmod +x /tmp/create_stub_so.sh
EOF

# Создаем DEX файл-заглушку
echo "[2/4] Creating stub DEX file..."

# Простой Java класс
cat > /tmp/StubClass.java << 'EOF'
package com.modmenu.loader;

import android.content.Context;

public class StubClass {
    public static void init(Context context) {
        // Stub implementation
    }
}
EOF

echo "[3/4] Compiling Java stub..."

# Компилируем Java без Android SDK (используем только java.lang)
javac -d build/classes /tmp/StubClass.java 2>/dev/null || {
    echo "Creating minimal class file manually..."
    # Создаем минимальный class файл
    python3 << 'PYEOF'
import struct
import os

# Создаем директорию
os.makedirs('build/classes/com/modmenu/loader', exist_ok=True)

# Минимальный class файл для com.modmenu.loader.StubClass
class_data = bytearray([
    0xCA, 0xFE, 0xBA, 0xBE,  # magic
    0x00, 0x00, 0x00, 0x34,  # version
    0x00, 0x0D,              # constant_pool_count
    # constant pool entries would go here
    0x00, 0x21,              # access_flags (ACC_PUBLIC | ACC_SUPER)
    0x00, 0x01,              # this_class
    0x00, 0x07,              # super_class
    # ... more class file structure
])

with open('build/classes/com/modmenu/loader/StubClass.class', 'wb') as f:
    f.write(class_data)
PYEOF
}

echo "[4/4] Creating final artifacts..."

# Создаем пустой файл для демонстрации
echo "Stub library for mod menu" > libs/arm64-v8a/libmodmenu_noroot.so
echo "Stub library for mod menu" > libs/armeabi-v7a/libmodmenu_noroot.so
echo "Stub DEX file for mod menu" > build/modmenu.dex

echo ""
echo "=========================================="
echo "  Build complete!"
echo "=========================================="
echo ""
echo "Created stub files:"
echo "  libs/arm64-v8a/libmodmenu_noroot.so"
echo "  libs/armeabi-v7a/libmodmenu_noroot.so"
echo "  build/classes/com/modmenu/loader/StubClass.class"
echo "  build/modmenu.dex"
echo ""
echo "NOTE: These are stub files for demonstration."
echo "To build real components, you need:"
echo "  - Android NDK (r21e or newer)"
echo "  - Android SDK"
echo "  - Proper build environment"
echo ""