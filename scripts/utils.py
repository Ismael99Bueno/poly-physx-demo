import os
import subprocess
import shutil
import glob
from typing import Callable


def clean_ppx(root_path: str, /) -> None:
    print("Removing build files for poly-physx...")
    __remove(f"{root_path}/*/bin")
    __remove(f"{root_path}/vendor/*/bin")
    __remove(f"{root_path}/*/build")
    __remove(f"{root_path}/vendor/*/build")
    __remove(f"{root_path}/*/Makefile", os.remove)
    __remove(f"{root_path}/vendor/*/Makefile", os.remove)
    if os.path.exists(f"{root_path}/Makefile"):
        os.remove(f"{root_path}/Makefile")

    print("Done.\n")


def clean_sfml(root_path: str, /) -> None:
    print("Removing build files for SFML...")
    build_sfml_path = f"{root_path}/vendor/SFML/build-sfml"
    try:
        shutil.rmtree(build_sfml_path)
    except FileNotFoundError:
        print(
            f"Attempted to remove {build_sfml_path} directory, but it does not exist. Skipping..."
        )
    print("Done.\n")


def clean_all(root_path: str, /) -> None:
    clean_sfml(root_path)
    clean_ppx(root_path)


def build_ppx(root_path: str, /) -> None:
    clean_ppx(root_path)
    print("Generating build files for poly-physx...")
    subprocess.run(["premake5", f"--file={root_path}/premake5.lua", "gmake"])


def build_sfml(root_path: str, /) -> None:
    clean_sfml(root_path)
    print("Generating build files for SFML...")

    sfml_path = f"{root_path}/vendor/SFML"
    if not os.path.exists(sfml_path):
        raise FileNotFoundError(
            f"{sfml_path} not found. Did you pass the source path correctly?"
        )
    build_sfml_path = f"{root_path}/vendor/SFML/build-sfml"
    os.mkdir(build_sfml_path)
    subprocess.run(
        ["cmake", "-S", sfml_path, "-B", build_sfml_path, "-DCMAKE_BUILD_TYPE=Release"]
    )
    subprocess.run(["cmake", "--build", build_sfml_path])
    print("Done.\n")


def build_all(root_path: str, /) -> None:
    build_sfml(root_path)
    build_ppx(root_path)


def __remove(pattern: str, remfunc: Callable[[str], None] = shutil.rmtree) -> None:
    for file in glob.glob(pattern):
        print(f"Removing {file}...")
        remfunc(file)
