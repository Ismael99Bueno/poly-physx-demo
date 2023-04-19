import sys
from exceptions import PythonVersionError, PackageNotFoundError
import importlib.util
import subprocess
import os
from utils import Buddy
from typing import Union
import time


def validate_python_version(
    req_major: int = 3, req_minor: int = 5, req_micro: int = 0
) -> None:
    print("\n==== PYTHON VALIDATION ====")

    major, minor, micro = (
        sys.version_info.major,
        sys.version_info.minor,
        sys.version_info.micro,
    )
    for required, current in zip(
        [req_major, req_minor, req_micro], [major, minor, micro]
    ):
        if current < required:
            raise PythonVersionError(
                f"{req_major}.{req_minor}.{req_micro}", f"{major}.{minor}.{micro}"
            )
        elif current > required:
            print(f"Valid python version detected: {major}.{minor}.{micro}")
            return
    print(f"Valid python version detected: {major}.{minor}.{micro}")


def validate_python_packages(package_names: Union[str, list[str]]) -> None:
    if isinstance(package_names, str):
        package_names = [package_names]
    needs_restart = False
    for package in package_names:
        needs_restart = not __validate_package(package) or needs_restart

    if needs_restart:
        __restart_to_apply_changes()


def __validate_package(package_name: str) -> bool:
    if (
        package_name in sys.modules
        and importlib.util.find_spec(package_name) is not None
    ):
        print(f"Package '{package_name}' installed\n")
        return True

    if not __install_python_package(package_name):
        raise PackageNotFoundError(package_name)

    print(f"Package '{package_name}' installed\n")
    return False


def __install_python_package(package_name: str) -> bool:
    if not Buddy().prompt_to_install(f"'{package_name}' package"):
        return False

    print(f"Starting '{package_name}' package installation...")
    subprocess.run(
        ["python", "-m", "pip", "install", package_name],
        check=True,
    )
    return True


def __restart_to_apply_changes() -> None:
    print("Script will now execute again for the changes to take effect")
    time.sleep(2.0)
    os.startfile(f"{Buddy().root_path}/scripts/setup-win.bat")
    exit()
