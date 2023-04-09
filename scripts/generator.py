import os
import subprocess
import shutil
import glob
from typing import Callable
from abc import ABC, abstractmethod
import platform
from exceptions import PathNotFoundError
from utils import ROOT_PATH


class Generator(ABC):
    @abstractmethod
    def build(self) -> None:
        ...

    @abstractmethod
    def clean(self) -> None:
        ...


class PPXGenerator(Generator):
    def __init__(self, action: str) -> None:
        self.__action = action

    def build(self) -> None:
        print("Generating build files for poly-physx...")
        subprocess.run(
            [
                (
                    "premake5"
                    if platform.system() == "Darwin"
                    else f"{ROOT_PATH}/vendor/premake/bin/premake.exe"
                ),
                f"--file={ROOT_PATH}/premake5.lua",
                self.__action,
            ]
        )

    def clean(self) -> None:
        print("Removing build files for poly-physx...")
        PPXGenerator.__remove(f"{ROOT_PATH}/*/bin")
        PPXGenerator.__remove(f"{ROOT_PATH}/vendor/*/bin")
        PPXGenerator.__remove(f"{ROOT_PATH}/*/build")
        PPXGenerator.__remove(f"{ROOT_PATH}/vendor/*/build")
        PPXGenerator.__remove(f"{ROOT_PATH}/*/Makefile", os.remove)
        PPXGenerator.__remove(f"{ROOT_PATH}/vendor/*/Makefile", os.remove)
        if os.path.exists(f"{ROOT_PATH}/Makefile"):
            os.remove(f"{ROOT_PATH}/Makefile")

        print("Done.\n")

    @staticmethod
    def __remove(pattern: str, remfunc: Callable[[str], None] = shutil.rmtree) -> None:
        for file in glob.glob(pattern):
            print(f"Removing {file}...")
            remfunc(file)


class SFMLGenerator(Generator):
    def build(self) -> None:
        self.clean()
        print("Generating build files for SFML...")

        sfml_path = f"{ROOT_PATH}/vendor/SFML"
        if not os.path.exists(sfml_path):
            raise PathNotFoundError(sfml_path)
        build_sfml_path = f"{ROOT_PATH}/vendor/SFML/build-sfml"
        os.mkdir(build_sfml_path)

        mac_ver, _, arch = platform.mac_ver()
        subprocess.run(
            [
                "cmake",
                (
                    "-GUnix Makefiles"
                    if platform.system() == "Darwin"
                    else "-GMinGW Makefiles"
                ),
                "-S",
                sfml_path,
                "-B",
                build_sfml_path,
                "-DCMAKE_BUILD_TYPE=Release",
                f"-DCMAKE_OSX_ARCHITECTURES={arch}",
                f"-DCMAKE_OSX_DEPLOYMENT_TARGET={mac_ver.split('.')[0]}",
            ]
        )
        subprocess.run(["cmake", "--build", build_sfml_path])
        print("Done.\n")

    def clean(self) -> None:
        print("Removing build files for SFML...")
        build_sfml_path = f"{ROOT_PATH}/vendor/SFML/build-sfml"
        try:
            shutil.rmtree(build_sfml_path)
        except FileNotFoundError:
            print(
                f"Attempted to remove {build_sfml_path} directory, but it does not exist. Skipping..."
            )
        print("Done.\n")


class FullGenerator(Generator):
    def __init__(self, action: str) -> None:
        self.__ppx_gen = PPXGenerator(action)
        self.__sfml_gen = SFMLGenerator()

    def build(self) -> None:
        self.__sfml_gen.build()
        self.__ppx_gen.build()

    def clean(self) -> None:
        self.__ppx_gen.clean()
        self.__sfml_gen.clean()
