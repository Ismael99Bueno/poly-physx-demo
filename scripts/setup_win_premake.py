from utils import Buddy
import os
from utils import download_file, unzip_file
from exceptions import DependencyNotFoundError


def validate_premake() -> None:
    print("\n==== PREMAKE VALIDATION ====")

    dir = f"{Buddy().root_path}/vendor/premake/bin"
    if not __is_premake_installed(dir) and not __install_premake(dir):
        raise DependencyNotFoundError("Premake")
    print("Premake installed")


def __is_premake_installed(dir: str) -> bool:
    return os.path.exists(f"{dir}/premake5.exe")


def __install_premake(dir: str) -> bool:
    version = "5.0.0-beta2"
    zip_url = f"https://github.com/premake/premake-core/releases/download/v{version}/premake-{version}-windows.zip"
    license_url = (
        "https://raw.githubusercontent.com/premake/premake-core/master/LICENSE.txt"
    )
    premake_zip_path = f"{dir}/premake-{version}-windows.zip"

    if not Buddy().prompt_to_install("Premake"):
        return False

    print(f"Starting premake installation...")
    print(f"Downloading {zip_url} to {premake_zip_path}...")
    download_file(zip_url, premake_zip_path)
    print(f"\nExtracting {premake_zip_path}...")
    unzip_file(premake_zip_path, dir)
    print(f"\nDownloading premake license from {license_url}...")
    download_file(license_url, f"{dir}/LICENSE.txt")
    print(
        f"\nPremake has been successfully installed. This installation is local. Removing the project or executing generate_build_files.py --clean will get rid of the installation."
    )

    return __is_premake_installed(dir)
