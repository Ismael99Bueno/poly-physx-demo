from utils import ROOT_PATH
import os
from utils import download_file
import subprocess
from exceptions import DependencyNotFoundError


def validate_cmake() -> None:
    if not __is_cmake_installed() and not __install_cmake():
        raise DependencyNotFoundError("CMake")


def __is_cmake_installed() -> bool:
    return subprocess.run(["cmake", "--version"], capture_output=True).returncode == 0


def __install_cmake() -> bool:
    dir = f"{ROOT_PATH}/vendor/CMake/bin"
    version = "3.26.3"
    arch = "x86_64"  # platform.machine()
    installer_url = f"https://github.com/Kitware/CMake/releases/download/v{version}/cmake-{version}-windows-{arch}.msi"
    installer_path = f"{dir}/cmake-{version}-windows-{arch}.msi"

    while True:
        answer = input("CMake installation not found. Do you wish to install? [Y]/N ")
        if answer == "n" or answer == "N":
            return False
        elif answer == "y" or answer == "Y" or answer == "":
            break

    print("Installing CMake...")
    print(f"Downloading {installer_url} to {installer_path}...")
    download_file(installer_url, installer_path)
    print(
        "CMake installer will now be executed. When prompted by the installation MAKE SURE TO ADD CMAKE TO PATH. Once the installation completes, RE-RUN the script"
    )
    input("\nPress any key to continue...")
    os.startfile(installer_path)
    exit()
