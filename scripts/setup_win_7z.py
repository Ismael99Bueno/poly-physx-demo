from exceptions import DependencyNotFoundError
import subprocess
from utils import Buddy, download_file
import os


def validate_7z() -> None:
    print("\n==== 7z VALIDATION ====")

    if not __resolve_7z_installation() and not __install_7z():
        raise DependencyNotFoundError("7z")
    print("7z installed")


def __is_7z_installed() -> bool:
    return subprocess.run("7z", shell=True, capture_output=True).returncode == 0


def __resolve_7z_installation() -> bool:
    if __is_7z_installed():
        return True

    print("7z installation not found in path")
    bud = Buddy()
    if os.path.exists(bud.default_7z_path):
        print(f"7z installation found at {bud.default_7z_path}. Adding to path...")
        bud.add_to_path_with_binaries(bud.default_7z_path)
        if __is_7z_installed():
            return True
        print("Failed. Still unable to find 7z executable in path.")

    if bud.prompt(
        f"7z installation not found at {bud.default_7z_path}. Is it located elsewhere?"
    ):
        sz_path = input(
            "Enter 7z installation path (Use forward or double backward slashes): "
        )
        print(f"Adding {sz_path} to path...")
        bud.add_to_path_with_binaries(sz_path)
        return __is_7z_installed()
    return False


def __install_7z() -> bool:
    bud = Buddy()

    dir = f"{bud.root_path}/vendor/7z/bin"
    installer_name = "7z2201.exe" if bud.os_architecture == "x86" else "7z2201-x64.exe"
    installer_url = f"https://www.7-zip.org/a/{installer_name}"
    installer_path = f"{dir}/{installer_name}"

    if not bud.prompt_to_install("7z"):
        return False

    print("Starting 7z installation...")
    print(f"Downloading {installer_url} to {installer_path}...")
    download_file(installer_url, installer_path)
    print("Executing silent installation...")
    subprocess.run([installer_path, "/S"], shell=True, check=True)

    bud.add_to_path_with_binaries(bud.default_7z_path)
    print(f"\n7z has been successfully installed at {bud.default_7z_path}.")

    return __is_7z_installed()
