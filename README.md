# Android Application Artifacts Repository

## Overview

This repository contains decompiled Android application artifacts from a Unity-based game. The repository includes binary Android components, native libraries, and application resources extracted from an APK.

## Repository Contents

### Core Android Components
- **AndroidManifest.xml** - Binary Android manifest (AXML format)
- **classes.dex through classes8.dex** - Dalvik bytecode files containing application code
- **META-INF/** - Manifest and signing information

### Native Libraries
Located in `lib/arm64-v8a/`:
- **libmain.so** - Unity IL2CPP runtime
- **lib_burst_generated.so** - Unity Burst compiler output
- **libphonon.so** - Audio system
- **libVivoxNative.so** - Vivox voice communications SDK
- **libFirebaseCpp\*.so** - Firebase C++ SDKs (Analytics, Crashlytics, Messaging, RemoteConfig)
- **libfmod.so, libfmodstudio.so** - FMOD audio engine
- **libaudioplugin_phonon.so** - Phonon audio plugin
- Various ad SDK native libraries (AppLovin, etc.)

### Assets
- **Unity game resources** in `assets/bin/`
- **Ad SDK JavaScript files** (MRAID bridge, OMID viewers)
- **Ad mediation resources** (Facebook, AppLovin, Yandex, etc.)
- **Game configuration files** (GameSettings.prop, builddatas.json)

### Protocol Buffers
- **client_analytics.proto** - Firebase client analytics schema
- **messaging_event.proto** - Firebase messaging event schema
- **messaging_event_extension.proto** - Extended messaging event schema

## Technology Stack

### Core
- **Unity Engine** with IL2CPP runtime
- **Android** native development

### Analytics & Monitoring
- **Firebase Analytics**
- **Firebase Crashlytics**
- **Firebase Remote Config**
- **AppMetrica** (Yandex Analytics)

### Communications
- **Firebase Cloud Messaging**
- **Vivox** voice SDK

### Monetization
- **AppLovin MAX** mediation
- **IronSource/LevelPlay**
- **Google Mobile Ads (AdMob)**
- **Facebook Audience Network**
- **Unity Ads**
- **Multiple other ad networks** (Yandex, Fyber, Moloco, Mintegral, Vungle)

### Other SDKs
- **Google Play Integrity**
- **VK ID** authentication
- **Intercom** customer support

## Important Notes

### File Formats
- **AndroidManifest.xml** is in binary AXML format, not plain text
- **DEX files** contain compiled Dalvik bytecode
- **Native libraries** (.so files) are compiled binaries
- Most configuration files are in their original binary formats

### Analysis Limitations
This repository contains **decompiled artifacts only**:
- No source code is available
- No build system (Gradle, etc.) is present
- Files are read-only artifacts from APK extraction

### Security Considerations
- This repository contains production application binaries
- API keys and sensitive data may be present in the compiled code
- Native libraries may contain proprietary code

## Working with This Repository

### Viewing Files
- Text files: Use standard text editors
- Binary files: Use hex editors or specialized tools (e.g., `aapt` for AndroidManifest)
- DEX files: Use tools like `dex2jar`, `jadx`, or `apktool`

### Analyzing Components
- **AndroidManifest**: `aapt dump xmltree <apk-file> AndroidManifest.xml`
- **DEX files**: `jadx classes.dex` (decompiles to Java)
- **Native libraries**: `strings libname.so` or use disassemblers like Ghidra/IDA

## Repository Structure

```
.
├── AndroidManifest.xml       # Binary Android manifest
├── classes*.dex             # Application bytecode (8 files)
├── lib/
│   └── arm64-v8a/         # ARM64 native libraries
├── assets/                 # Application resources
│   ├── bin/                # Unity game data
│   ├── *.js                # Ad SDK JavaScript
│   └── *.html              # Ad viewer templates
├── META-INF/               # Signing and metadata
├── *.proto                 # Protocol buffer definitions
└── *.properties            # SDK version properties
```

## License

This repository contains decompiled application artifacts. The original application's license terms apply to these materials.

## Contact

For questions about this repository, please refer to the original application's documentation or support channels.
