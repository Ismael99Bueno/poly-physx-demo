import os
import subprocess
import shutil
import glob
from typing import Callable
from abc import ABC, abstractmethod
import platform
from exceptions import PathNotFoundError
from utils import ROOT_PATH, PREMAKE_TO_CMAKE_GEN


class Generator(ABC):
    def __init__(self, generator: str) -> None:
        super().__init__()
        self._generator = generator

    @abstractmethod
    def build(self) -> None:
        ...

    @abstractmethod
    def clean(self) -> None:
        ...


class PPXGenerator(Generator):
    def __init__(self, generator: str) -> None:
        super().__init__(generator)

    def build(self) -> None:
        print("Generating build files for poly-physx...")
        subprocess.run(
            [
                (
                    "premake5"
                    if platform.system() == "Darwin"
                    else f"{ROOT_PATH}/vendor/premake/bin/premake5.exe"
                ),
                f"--file={ROOT_PATH}/premake5.lua",
                self._generator,
            ],
            shell=platform.system() == "Windows",
        )

    def clean(self) -> None:
        print("Removing build, binary and project files..")
        PPXGenerator.__remove(f"{ROOT_PATH}/*/bin")
        PPXGenerator.__remove(f"{ROOT_PATH}/vendor/*/bin")
        PPXGenerator.__remove(f"{ROOT_PATH}/**/build")
        PPXGenerator.__remove(f"{ROOT_PATH}/**/Makefile", os.remove)
        PPXGenerator.__remove(f"{ROOT_PATH}/**/*.vcxproj*", os.remove)
        PPXGenerator.__remove(f"{ROOT_PATH}/**/*.sln", os.remove)
        PPXGenerator.__remove(f"{ROOT_PATH}/*.sln", os.remove)
        if os.path.exists(f"{ROOT_PATH}/Makefile"):
            os.remove(f"{ROOT_PATH}/Makefile")

        print("Done.\n")

    @staticmethod
    def __remove(pattern: str, remfunc: Callable[[str], None] = shutil.rmtree) -> None:
        for file in glob.glob(pattern):
            print(f"Removing {file}...")
            remfunc(file)


class SFMLGenerator(Generator):
    def __init__(self, generator: str) -> None:
        super().__init__(generator)

    def build(self) -> None:
        self.clean()
        print("Generating build files for SFML...")

        sfml_path = f"{ROOT_PATH}/vendor/SFML"
        if not os.path.exists(sfml_path):
            raise PathNotFoundError(sfml_path)
        build_sfml_path = f"{ROOT_PATH}/vendor/SFML/build-sfml"
        os.mkdir(build_sfml_path)

        mac_ver, _, arch = platform.mac_ver()
        if platform.system() == "Windows":
            os.environ["PATH"] += os.pathsep + os.pathsep.join(
                ["C:\\MinGW\\bin"]
            )  # PUT THIS ELSEWHERE

        is_macos = platform.system() == "Darwin"

        is_visual_studio = not is_macos and self._generator.startswith("vs")
        premake_to_cmake_vs = (
            PREMAKE_TO_CMAKE_GEN[self._generator]
            if is_visual_studio
            else "MinGW Makefiles"
        )
        subprocess.run(
            [
                "cmake",
                ("-GUnix Makefiles" if is_macos else f"-G{premake_to_cmake_vs}"),
                "-S",
                sfml_path,
                "-B",
                build_sfml_path,
                "-DCMAKE_BUILD_TYPE=Release",
                f"-DCMAKE_OSX_ARCHITECTURES={arch}",
                f"-DCMAKE_OSX_DEPLOYMENT_TARGET={mac_ver.split('.')[0]}",
                "-DWARNINGS_AS_ERRORS=FALSE",
                f"-DBUILD_SHARED_LIBS={'TRUE' if is_macos else 'FALSE'}",
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
    def __init__(self, generator: str) -> None:
        super().__init__(generator)
        self.__ppx_gen = PPXGenerator(generator)
        self.__sfml_gen = SFMLGenerator(generator)

    def build(self) -> None:
        self.__sfml_gen.build()
        self.__ppx_gen.build()

    def clean(self) -> None:
        self.__ppx_gen.clean()
        self.__sfml_gen.clean()
