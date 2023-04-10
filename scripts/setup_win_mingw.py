from utils import Buddy, download_file
import os
import subprocess
from exceptions import DependencyNotFoundError


def validate_mingw() -> None:
    print("====MINGW VALIDATION====")

    if not __is_mingw_installed() and not __install_mingw():
        raise DependencyNotFoundError("MinGW")
    print("MinGW installed")


def __is_mingw_installed() -> bool:
    bud = Buddy()
    if not os.path.exists(bud.mingw_path):
        return False

    bud.add_mingw_to_path()
    return __is_gxx_installed() and __is_make_installed()


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
    installer_url = (
        "https://osdn.net/frs/redir.php?m=nchc&f=mingw%2F68260%2Fmingw-get-setup.exe"
    )
    installer_path = f"{dir}/mingw-get-setup.exe"

    if not bud.prompt_to_install("MinGW"):
        return False

    print("Starting MinGW installation...")
    print(f"Downloading {installer_url} to {installer_path}...")
    download_file(installer_url, installer_path)
    print(
        "\nMinGW installer will now be executed. DO NOT CHECK THE MINGW GUI OPTION INSTALLATION (it won't be necessary for this setup)"
    )
    input("Press any key to begin installation...")
    os.startfile(installer_path)
    input("Press any key once the installation has finished...")
    os.startfile(f"{bud.root_path}/scripts/setup-win.bat")
    exit()


def __install_mingw_packages() -> bool:
    bud = Buddy()

    if not __is_gxx_installed():
        if not bud.prompt_to_install("g++"):
            return False

        print("Starting g++ installation...")
        subprocess.run(
            ["mingw-get", "install", "g++"], shell=True, capture_output=True, check=True
        )

    if not __is_make_installed():
        if not bud.prompt_to_install("make"):
            return False

        print("Starting make installation...")
        subprocess.run(
            ["mingw-get", "install", "mingw32-make"],
            shell=True,
            capture_output=True,
            check=True,
        )

    return __is_mingw_installed()
