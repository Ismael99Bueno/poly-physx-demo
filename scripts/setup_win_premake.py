from utils import ROOT_PATH
import os
from utils import download_file, unzip_file
from exceptions import DependencyNotFoundError


def validate_premake() -> None:
    print("\n====PREMAKE VALIDATION====")

    dir = f"{ROOT_PATH}/vendor/premake/bin"
    if not __is_premake_installed(dir) and not __install_premake(dir):
        raise DependencyNotFoundError("Premake")


def __is_premake_installed(dir: str) -> bool:
    return os.path.exists(f"{dir}/premake5.exe")


def __install_premake(dir: str) -> bool:
    version = "5.0.0-beta2"
    zip_url = f"https://github.com/premake/premake-core/releases/download/v{version}/premake-{version}-windows.zip"
    license_url = (
        "https://raw.githubusercontent.com/premake/premake-core/master/LICENSE.txt"
    )
    premake_zip_path = f"{dir}/premake-{version}-windows.zip"

    while True:
        answer = input("Premake installation not found. Do you wish to install? [Y]/N ")
        if answer == "n" or answer == "N":
            return False
        elif answer == "y" or answer == "Y" or answer == "":
            break

    print(f"Installing premake...")
    print(f"Downloading {zip_url} to {premake_zip_path}...")
    download_file(zip_url, premake_zip_path)
    print(f"Extracting {premake_zip_path}...")
    unzip_file(premake_zip_path, dir)
    print(f"Downloading premake license from {license_url}...")
    download_file(license_url, f"{dir}/LICENSE.txt")
    print(f"Premake has been successfully downloaded")

    return __is_premake_installed(dir)
