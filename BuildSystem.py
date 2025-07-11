import os
import subprocess
import platform
import configparser

ENGINE_VERSION = "1.0.0"
BUILD_DIR = "build"
INSTALL_DIR = f"FelissEngine_{ENGINE_VERSION}"

config = configparser.ConfigParser()
config.read("configuration.ini")

AA_MODE = config.get("Graphics", "AntiAliasing", fallback="TAA")
print(f"[CONFIG] Anti-Aliasing Mode: {AA_MODE}")

os.makedirs(BUILD_DIR, exist_ok=True)
os.chdir(BUILD_DIR)

print("[CMAKE] Configuring project...")
subprocess.run(["cmake", "..", f"-DENGINE_VERSION={ENGINE_VERSION}"])

print("[CMAKE] Building project...")
subprocess.run(["cmake", "--build", ".", "--config", "Release"])

print("[CMAKE] Installing to release folder...")
subprocess.run(["cmake", "--install", ".", "--prefix", f"../{INSTALL_DIR}"])

if platform.system() == "Windows":
    print("[PACKAGE] Creating setup.exe using Inno Setup (if installed)...")
    inno_script = "../InstallerScript.iss"
    if os.path.exists(inno_script):
        subprocess.run(["ISCC", inno_script])
    else:
        print("[WARN] Inno Setup script not found, skipping installer packaging.")

print(f"[DONE] FelissEngine v{ENGINE_VERSION} built and installed to {INSTALL_DIR}")
