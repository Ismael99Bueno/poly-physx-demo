import sys
import time
import importlib.util
import subprocess
import os

from validation.exceptions import PythonVersionError, PackageNotFoundError
from utility.buddy import Buddy
from pathlib import Path


def validate_python_version(
    req_major: int = 3, req_minor: int = 8, req_micro: int = 8
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
                f"Python version required: '{req_major}.{req_minor}.{req_micro}'",
                f"Python version found: '{major}.{minor}.{micro}'",
            )
        elif current > required:
            print(f"Valid python version detected: '{major}.{minor}.{micro}'")
            return
    print(f"Valid python version detected: '{major}.{minor}.{micro}'")


def validate_python_packages(package_names: str | list[str]) -> None:
    if isinstance(package_names, str):
        package_names = [package_names]
    needs_restart = False

    for package in package_names:
        needs_restart = not __validate_package(package) or needs_restart

    return needs_restart


def restart_to_apply_changes(path_to_script_to_rerun: Path) -> None:
    print("Script will now execute again for the changes to take effect")
    time.sleep(2.0)
    os.startfile(path_to_script_to_rerun)
    quit()


def __validate_package(package_name: str) -> bool:
    if importlib.util.find_spec(package_name) is not None:
        print(f"Package '{package_name}' installed\n")
        return True

    if not __install_python_package(package_name):
        raise PackageNotFoundError(f"The package '{package_name}' was not found")

    print(f"Package '{package_name}' installed\n")
    return False


def __install_python_package(package_name: str) -> bool:
    bud = Buddy()
    if not bud.all_yes and not bud.prompt_to_install(f"'{package_name}' package"):
        return False

    print(f"Starting '{package_name}' package installation...")
    subprocess.run(
        ["python", "-m", "pip", "install", package_name],
        check=True,
    )
    return True
