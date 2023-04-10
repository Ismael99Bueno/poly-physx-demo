from utils import Buddy
import os
from utils import download_file
import subprocess
from exceptions import DependencyNotFoundError


def validate_cmake() -> None:
    print("\n====CMAKE VALIDATION====")

    if not __is_cmake_installed() and not __install_cmake():
        raise DependencyNotFoundError("CMake")
    print("CMake installed")


def __is_cmake_installed() -> bool:
    return (
        subprocess.run(
            ["cmake", "--version"], shell=True, capture_output=True
        ).returncode
        == 0
    )


def __install_cmake() -> bool:
    bud = Buddy()

    dir = f"{bud.root_path}/vendor/CMake/bin"
    version = "3.26.3"
    arch = "x86_64"  # platform.machine()
    installer_url = f"https://github.com/Kitware/CMake/releases/download/v{version}/cmake-{version}-windows-{arch}.msi"
    installer_path = f"{dir}/cmake-{version}-windows-{arch}.msi"

    if not bud.prompt_to_install("CMake"):
        return False

    print("Starting CMake installation...")
    print(f"Downloading {installer_url} to {installer_path}...")
    download_file(installer_url, installer_path)
    print(
        "\nCMake installer will now be executed. When prompted by the installation MAKE SURE TO ADD CMAKE TO PATH"
    )
    input("Press any key to begin installation...")
    os.startfile(installer_path)
    input(
        "Press any key once the installation has finished. If the same installation prompt for CMake appears, cancel and re-run the script"
    )
    os.startfile(f"{bud.root_path}/scripts/setup-win.bat")
    exit()
