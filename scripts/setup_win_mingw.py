from utils import Buddy, download_file, unzip_file
import os
import subprocess
from exceptions import DependencyNotFoundError


def validate_mingw() -> None:
    print("\n==== MINGW VALIDATION ====")

    if not __is_mingw_installed() and not __install_mingw():
        raise DependencyNotFoundError("MinGW")
    print("MinGW installed")


def __is_mingw_installed() -> bool:
    bud = Buddy()
    if not os.path.exists(bud.mingw_path):
        return False
    bud.add_mingw_to_path()

    if not __is_gxx_installed() or not __is_make_installed():
        raise FileNotFoundError(
            f"It seems you have MinGW installed, but the g++ and/or make binaries where not found. Install them to proceed. To install them automatically by the script, you will have to remove your current mingw installation located at {bud.mingw_path}"
        )

    return True


def __is_gxx_installed() -> bool:
    return (
        subprocess.run(["g++", "--version"], shell=True, capture_output=True).returncode
        == 0
    )


def __is_make_installed() -> bool:
    return (
        subprocess.run(
            ["mingw32-make", "--version"], shell=True, capture_output=True
        ).returncode
        == 0
    )


def __install_mingw() -> bool:
    bud = Buddy()
    if os.path.exists(bud.mingw_path):
        return __install_mingw_packages()

    dir = f"{Buddy().root_path}/vendor/MinGW/bin"
    arch = "i686" if bud.os_architecture == "x86" else bud.os_architecture
    thingy = "dwarf" if bud.os_architecture == "x86" else "sjlj"

    zip_name = f"{arch}-8.1.0-release-posix-{thingy}-rt_v6-rev0.7z"
    zip_url = f"https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win{'32' if bud.os_architecture == 'x86' else '64'}/Personal%20Builds/mingw-builds/8.1.0/threads-posix/{thingy}/{zip_name}"

    zip_path = f"{dir}/{zip_name}"

    if not bud.prompt_to_install("MinGW"):
        return False

    print("Starting MinGW installation...")
    print(f"Downloading {zip_url} to {zip_path}...")
    download_file(zip_url, zip_path)
    print(f"\nExtracting {zip_path}...")
    os.makedirs(bud.mingw_path)
    unzip_file(zip_path, bud.mingw_path)
    print(f"\MinGW has been successfully downloaded")

    return __is_mingw_installed()


def __install_mingw_packages() -> bool:
    bud = Buddy()

    if not __is_gxx_installed():
        if not bud.prompt_to_install("g++"):
            return False

        print("Starting g++ installation...")
        subprocess.run(["mingw-get", "install", "g++"], shell=True, check=True)

    print("\n")
    if not __is_make_installed():
        if not bud.prompt_to_install("make"):
            return False

        print("Starting make installation...")
        subprocess.run(
            ["mingw-get", "install", "mingw32-make"],
            shell=True,
            check=True,
        )

    return __is_mingw_installed()
