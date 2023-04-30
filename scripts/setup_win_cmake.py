from utils import Buddy
import os
from utils import download_file
import subprocess
from exceptions import DependencyNotFoundError


def validate_cmake() -> None:
    print("\n==== CMAKE VALIDATION ====")

    if not __resolve_cmake_installation() and not __install_cmake():
        raise DependencyNotFoundError("CMake")
    print("CMake installed")


def __is_cmake_installed() -> bool:
    return (
        subprocess.run(
            ["cmake", "--version"], shell=True, capture_output=True
        ).returncode
        == 0
    )


def __resolve_cmake_installation() -> bool:
    if __is_cmake_installed():
        return True

    print("CMake installation not found in path")
    bud = Buddy()
    if os.path.exists(bud.cmake_path):
        print(f"CMake installation found at {bud.cmake_path}. Adding to path...")
        bud.add_to_path_with_binaries(bud.cmake_path)
        if __is_cmake_installed():
            return True
        print("Failed. Still unable to find CMake executable in path.")

    if bud.prompt(
        f"CMake installation not found at {bud.cmake_path}. Is it located elsewhere?",
        default=False,
    ):
        cmake_path = input(
            "Enter CMake installation path (Use forward or double backward slashes): "
        )
        print(f"Adding {cmake_path} to path...")
        bud.add_to_path_with_binaries(cmake_path)
        bud.cmake_path = cmake_path
        return __is_cmake_installed()
    return False


def __install_cmake() -> bool:
    bud = Buddy()

    dir = f"{bud.root_path}/vendor/CMake/bin"
    version = "3.26.3"
    arch = "i386" if bud.os_architecture == "x86" else bud.os_architecture
    installer_url = f"https://github.com/Kitware/CMake/releases/download/v{version}/cmake-{version}-windows-{arch}.msi"
    installer_path = f"{dir}/cmake-{version}-windows-{arch}.msi"

    if not bud.prompt_to_install("CMake"):
        return False

    print("Starting CMake installation...")
    print(f"Downloading {installer_url} to {installer_path}...")
    download_file(installer_url, installer_path)
    print(
        "\nCMake installer will now be executed. When prompted by the installation MAKE SURE TO ADD CMAKE TO PATH. Once the installation finishes, re-run the script"
    )
    input("Press any key to begin installation...")
    os.startfile(installer_path)
    exit(3)
