from utils import ROOT_PATH
import os
from utils import download_file
import subprocess
from exceptions import DependencyNotFoundError


def validate_mingw() -> None:
    print("====MINGW VALIDATION====")

    if not __is_mingw_installed() and not __install_mingw():
        raise DependencyNotFoundError("MinGW")
    print("MinGW installed")


def __is_mingw_installed() -> bool:
    if not os.path.exists("C:\\MinGW\\bin"):
        return False

    subprocess.run(["set", "PATH=%PATH%;C:\\MinGW\\bin"], shell=True)
    return (
        subprocess.run(["g++", "--version"], shell=True, capture_output=True).returncode
        == 0
    )


def __install_mingw() -> bool:
    dir = f"{ROOT_PATH}/vendor/MinGW/bin"
    installer_url = (
        "https://osdn.net/frs/redir.php?m=nchc&f=mingw%2F68260%2Fmingw-get-setup.exe"
    )
    installer_path = f"{dir}/mingw-get-setup.exe"

    while True:
        answer = input("MinGW installation not found. Do you wish to install? [Y]/N ")
        if answer == "n" or answer == "N":
            return False
        elif answer == "y" or answer == "Y" or answer == "":
            break

    print("Starting MinGW installation...")
    print(f"Downloading {installer_url} to {installer_path}...")
    download_file(installer_url, installer_path)
    print(
        "MinGW installer will now be executed. Once the installation completes, RE-RUN the script"
    )
    input("Press any key to execute installer...")
    os.startfile(installer_path)
    exit()
