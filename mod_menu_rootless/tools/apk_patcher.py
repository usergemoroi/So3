#!/usr/bin/env python3
"""
APK Patcher для интеграции мод меню без рута
Автоматически патчит APK и добавляет загрузчик мода
"""

import os
import sys
import shutil
import zipfile
import subprocess
import xml.etree.ElementTree as ET
from pathlib import Path

class APKPatcher:
    def __init__(self, apk_path):
        self.apk_path = Path(apk_path)
        self.work_dir = Path("apk_patch_work")
        self.output_apk = self.apk_path.parent / f"{self.apk_path.stem}_modded.apk"
        
    def extract_apk(self):
        """Извлекает содержимое APK"""
        print("[*] Extracting APK...")
        
        if self.work_dir.exists():
            shutil.rmtree(self.work_dir)
        
        self.work_dir.mkdir(parents=True)
        
        with zipfile.ZipFile(self.apk_path, 'r') as zip_ref:
            zip_ref.extractall(self.work_dir)
        
        print("[+] APK extracted successfully")
    
    def patch_manifest(self):
        """Патчит AndroidManifest.xml для добавления прав и сервиса"""
        print("[*] Patching AndroidManifest.xml...")
        
        manifest_path = self.work_dir / "AndroidManifest.xml"
        
        if not manifest_path.exists():
            print("[!] AndroidManifest.xml not found, skipping...")
            return
        
        try:
            subprocess.run([
                "apktool", "d", str(self.apk_path), 
                "-o", str(self.work_dir / "decoded"),
                "-f"
            ], check=True, capture_output=True)
            
            decoded_manifest = self.work_dir / "decoded" / "AndroidManifest.xml"
            
            tree = ET.parse(decoded_manifest)
            root = tree.getroot()
            
            ns = {'android': 'http://schemas.android.com/apk/res/android'}
            ET.register_namespace('android', ns['android'])
            
            permissions = [
                "android.permission.SYSTEM_ALERT_WINDOW",
                "android.permission.INTERNET",
                "android.permission.ACCESS_NETWORK_STATE"
            ]
            
            for perm in permissions:
                perm_elem = ET.Element('uses-permission')
                perm_elem.set('{http://schemas.android.com/apk/res/android}name', perm)
                root.insert(0, perm_elem)
            
            application = root.find('application')
            if application is not None:
                service = ET.SubElement(application, 'service')
                service.set('{http://schemas.android.com/apk/res/android}name', 
                           'com.modmenu.loader.OverlayService')
                service.set('{http://schemas.android.com/apk/res/android}enabled', 'true')
                service.set('{http://schemas.android.com/apk/res/android}exported', 'false')
            
            tree.write(decoded_manifest, encoding='utf-8', xml_declaration=True)
            
            print("[+] Manifest patched successfully")
            
        except Exception as e:
            print(f"[!] Failed to patch manifest: {e}")
    
    def inject_loader(self):
        """Внедряет загрузчик в классы DEX"""
        print("[*] Injecting mod loader into DEX...")
        
        dex_files = list(self.work_dir.glob("classes*.dex"))
        
        if not dex_files:
            print("[!] No DEX files found")
            return
        
        main_dex = self.work_dir / "classes.dex"
        
        decoded_dir = self.work_dir / "decoded"
        
        smali_dirs = list(decoded_dir.glob("smali*"))
        
        if not smali_dirs:
            print("[!] No smali directories found")
            return
        
        main_smali = smali_dirs[0]
        
        application_smali = self._find_application_class(main_smali)
        
        if application_smali:
            self._inject_loader_code(application_smali)
        else:
            print("[!] Application class not found, creating custom one...")
            self._create_custom_application(main_smali)
        
        print("[+] Loader injected successfully")
    
    def _find_application_class(self, smali_dir):
        """Находит главный класс Application"""
        for smali_file in smali_dir.rglob("*.smali"):
            try:
                with open(smali_file, 'r', encoding='utf-8', errors='ignore') as f:
                    content = f.read()
                    
                    if 'Landroid/app/Application;' in content and \
                       '.method public onCreate()V' in content:
                        print(f"[+] Found Application class: {smali_file}")
                        return smali_file
            except Exception as e:
                continue
        
        return None
    
    def _inject_loader_code(self, smali_file):
        """Внедряет код загрузки в метод onCreate"""
        print(f"[*] Injecting into {smali_file.name}...")
        
        with open(smali_file, 'r', encoding='utf-8') as f:
            lines = f.readlines()
        
        injection_code = '''
    invoke-static {p0}, Lcom/modmenu/loader/DexInjector;->inject(Landroid/content/Context;)V
    
    invoke-static {p0}, Lcom/modmenu/loader/ModMenuLoader;->inject(Landroid/app/Application;)V
'''
        
        new_lines = []
        injected = False
        
        for i, line in enumerate(lines):
            new_lines.append(line)
            
            if not injected and '.method public onCreate()V' in line:
                for j in range(i + 1, len(lines)):
                    new_lines.append(lines[j])
                    
                    if 'invoke-super' in lines[j] and 'onCreate' in lines[j]:
                        new_lines.append(injection_code)
                        injected = True
                        break
                
                if injected:
                    for k in range(j + 1, len(lines)):
                        new_lines.append(lines[k])
                    break
        
        if injected:
            with open(smali_file, 'w', encoding='utf-8') as f:
                f.writelines(new_lines)
            print("[+] Injection successful")
        else:
            print("[!] Failed to find injection point")
    
    def _create_custom_application(self, smali_dir):
        """Создает кастомный класс Application"""
        app_dir = smali_dir / "com" / "modmenu" / "custom"
        app_dir.mkdir(parents=True, exist_ok=True)
        
        app_smali = app_dir / "CustomApplication.smali"
        
        smali_content = '''.class public Lcom/modmenu/custom/CustomApplication;
.super Landroid/app/Application;

.method public constructor <init>()V
    .locals 0
    invoke-direct {p0}, Landroid/app/Application;-><init>()V
    return-void
.end method

.method public onCreate()V
    .locals 0
    
    invoke-super {p0}, Landroid/app/Application;->onCreate()V
    
    invoke-static {p0}, Lcom/modmenu/loader/DexInjector;->inject(Landroid/content/Context;)V
    
    invoke-static {p0}, Lcom/modmenu/loader/ModMenuLoader;->inject(Landroid/app/Application;)V
    
    return-void
.end method
'''
        
        with open(app_smali, 'w') as f:
            f.write(smali_content)
        
        print("[+] Custom Application class created")
    
    def copy_mod_files(self):
        """Копирует файлы мода в APK"""
        print("[*] Copying mod files...")
        
        assets_dir = self.work_dir / "assets"
        assets_dir.mkdir(exist_ok=True)
        
        mod_dex = Path("output/modmenu.dex")
        if mod_dex.exists():
            shutil.copy(mod_dex, assets_dir / "modmenu.dex")
            print("[+] Copied modmenu.dex")
        
        lib_arm64 = Path("output/libmodmenu_noroot.so")
        if lib_arm64.exists():
            lib_dir = self.work_dir / "lib" / "arm64-v8a"
            lib_dir.mkdir(parents=True, exist_ok=True)
            shutil.copy(lib_arm64, lib_dir / "libmodmenu_noroot.so")
            print("[+] Copied ARM64 library")
    
    def rebuild_apk(self):
        """Пересобирает APK"""
        print("[*] Rebuilding APK...")
        
        decoded_dir = self.work_dir / "decoded"
        
        try:
            subprocess.run([
                "apktool", "b", str(decoded_dir),
                "-o", str(self.output_apk)
            ], check=True, capture_output=True)
            
            print("[+] APK rebuilt successfully")
        except Exception as e:
            print(f"[!] Failed to rebuild APK: {e}")
    
    def sign_apk(self):
        """Подписывает APK"""
        print("[*] Signing APK...")
        
        try:
            unsigned_apk = self.output_apk
            signed_apk = self.apk_path.parent / f"{self.apk_path.stem}_modded_signed.apk"
            
            subprocess.run([
                "apksigner", "sign",
                "--ks", "debug.keystore",
                "--ks-pass", "pass:android",
                "--out", str(signed_apk),
                str(unsigned_apk)
            ], check=True, capture_output=True)
            
            print(f"[+] APK signed: {signed_apk}")
            
        except subprocess.CalledProcessError:
            print("[!] Signing failed, trying with jarsigner...")
            
            try:
                subprocess.run([
                    "jarsigner",
                    "-sigalg", "SHA1withRSA",
                    "-digestalg", "SHA1",
                    "-keystore", "debug.keystore",
                    "-storepass", "android",
                    str(self.output_apk),
                    "androiddebugkey"
                ], check=True)
                
                print("[+] APK signed with jarsigner")
            except Exception as e:
                print(f"[!] Signing failed: {e}")
    
    def cleanup(self):
        """Очищает временные файлы"""
        print("[*] Cleaning up...")
        
        if self.work_dir.exists():
            shutil.rmtree(self.work_dir)
        
        print("[+] Cleanup complete")
    
    def patch(self):
        """Выполняет полный цикл патчинга"""
        print("="*50)
        print("  APK Patcher - Rootless Mod Menu")
        print("="*50)
        print()
        
        try:
            self.extract_apk()
            self.patch_manifest()
            self.inject_loader()
            self.copy_mod_files()
            self.rebuild_apk()
            self.sign_apk()
            self.cleanup()
            
            print()
            print("="*50)
            print("  Patching complete!")
            print("="*50)
            print()
            print(f"Output: {self.output_apk}")
            print()
            print("Install the modded APK:")
            print(f"  adb install {self.output_apk}")
            print()
            
        except Exception as e:
            print(f"\n[!] Error: {e}")
            import traceback
            traceback.print_exc()
            sys.exit(1)

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 apk_patcher.py <path_to_apk>")
        sys.exit(1)
    
    apk_path = sys.argv[1]
    
    if not os.path.exists(apk_path):
        print(f"Error: APK not found: {apk_path}")
        sys.exit(1)
    
    patcher = APKPatcher(apk_path)
    patcher.patch()

if __name__ == "__main__":
    main()
