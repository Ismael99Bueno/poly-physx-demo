from pathlib import Path
from utility.buddy import Buddy
from utility.utils import download_file, unzip_file
from validation.exceptions import DependencyNotFoundError


def validate_premake() -> None:
    print("\n==== PREMAKE VALIDATION ====")

    premake_dir = Buddy().root_path / "vendor" / "premake" / "bin"
    if not __is_premake_installed(premake_dir) and not __install_premake(premake_dir):
        raise DependencyNotFoundError("The dependency 'premake' was not found")
    print("Premake installed")


def __is_premake_installed(premake_dir: Path) -> bool:
    return (premake_dir / "premake5.exe").exists()


def __install_premake(premake_dir: Path) -> bool:
    bud = Buddy()
    if not bud.all_yes and not bud.prompt_to_install("premake"):
        return False

    version = "5.0.0-beta2"
    zip_url = f"https://github.com/premake/premake-core/releases/download/v{version}/premake-{version}-windows.zip"
    license_url = (
        "https://raw.githubusercontent.com/premake/premake-core/master/LICENSE.txt"
    )
    zip_path = premake_dir / f"premake-{version}-windows.zip"

    print(f"Starting premake installation...")
    print(f"Downloading {zip_url} to {zip_path}...")
    download_file(zip_url, zip_path)
    print(f"\nExtracting {zip_path} to {premake_dir}...")
    unzip_file(zip_path, premake_dir)
    print(f"\nDownloading premake license from {license_url}...")
    download_file(license_url, f"{premake_dir}/LICENSE.txt")
    print(
        f"\nPremake has been successfully installed at {premake_dir}. This installation is local. Removing the project or executing generate_build_files.py --clean will get rid of the installation."
    )

    return __is_premake_installed(premake_dir)
