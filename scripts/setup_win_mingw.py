from utils import Buddy
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

    # SET THE MINGW PATH TO UTILS
    # SET ADD MINGW TO PATH TO UTILS
    os.environ["PATH"] += os.pathsep + os.pathsep.join(["C:\\MinGW\\bin"])
    return (
        subprocess.run(["g++", "--version"], shell=True, capture_output=True).returncode
        == 0  # ADD MAKE INSTALLATION mingw32-make
    )


def __install_mingw() -> bool:
    if os.path.exists(
        "C:\\MinGW\\bin"
    ):  # THIS SHOULD ALSO BE EXECUTED AT THE END OF THE FUNCTION
        subprocess.run(
            ["mingw-get", "install", "g++"], shell=True
        )  # Should already be in path
        subprocess.run(["mingw-get", "install", "make"], shell=True)
        return True  # False or error if the returncode is not 0

    dir = f"{Buddy().root_path}/vendor/MinGW/bin"
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
        "MinGW installer will now be executed. DO NOT CHECK THE MINGW GUI OPTION INSTALLATION (it won't be necessary for this setup). Once the installation completes, RE-RUN the script"
    )
    input("Press any key to execute installer...")
    os.startfile(installer_path)
    exit()
