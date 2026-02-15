#!/bin/bash

echo "=========================================="
echo "  Real-time Mod Debug Logs"
echo "=========================================="
echo ""
echo "Press Ctrl+C to stop"
echo ""

adb logcat -c

adb logcat | grep --line-buffered -E "ModMenu|OverlayService|DexInjector|IL2CPP|Protection|Hooks" | while read line; do
    if [[ $line == *"ERROR"* ]] || [[ $line == *"FATAL"* ]]; then
        echo -e "\033[31m$line\033[0m"
    elif [[ $line == *"WARNING"* ]]; then
        echo -e "\033[33m$line\033[0m"
    elif [[ $line == *"SUCCESS"* ]] || [[ $line == *"initialized"* ]]; then
        echo -e "\033[32m$line\033[0m"
    else
        echo "$line"
    fi
done
