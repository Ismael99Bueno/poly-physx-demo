import os
import subprocess
import shutil
import glob
from typing import Callable
from abc import ABC, abstractmethod
from exceptions import PathNotFoundError
from utils import Buddy


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

        bud = Buddy()
        subprocess.run(
            [
                (
                    "premake5"
                    if bud.is_macos
                    else f"{bud.root_path}/vendor/premake/bin/premake5.exe"
                ),
                f"--file={bud.root_path}/premake5.lua",
                self._generator,
            ],
            shell=bud.is_windows,
            check=True,
        )

    def clean(self) -> None:
        print("Removing build, binary and project files..")

        bud = Buddy()
        PPXGenerator.__remove(f"{bud.root_path}/*/bin")
        PPXGenerator.__remove(f"{bud.root_path}/vendor/*/bin")
        PPXGenerator.__remove(f"{bud.root_path}/**/build")
        PPXGenerator.__remove(f"{bud.root_path}/**/Makefile", os.remove)
        PPXGenerator.__remove(f"{bud.root_path}/**/*.vcxproj*", os.remove)
        PPXGenerator.__remove(f"{bud.root_path}/**/*.sln", os.remove)
        PPXGenerator.__remove(f"{bud.root_path}/*.sln", os.remove)
        if os.path.exists(f"{bud.root_path}/Makefile"):
            os.remove(f"{bud.root_path}/Makefile")

        print("Done.\n")

    @staticmethod
    def __remove(pattern: str, remfunc: Callable[[str], None] = shutil.rmtree) -> None:
        for file in glob.glob(pattern):
            print(f"Removed {file}")
            remfunc(file)


class SFMLGenerator(Generator):
    def __init__(self, generator: str) -> None:
        super().__init__(generator)

    def build(self) -> None:
        self.clean()
        print("Generating build files for SFML...")

        bud = Buddy()
        sfml_path = f"{bud.root_path}/vendor/SFML"
        if not os.path.exists(sfml_path):
            raise PathNotFoundError(sfml_path)
        build_sfml_path = f"{bud.root_path}/vendor/SFML/build-sfml"
        os.mkdir(build_sfml_path)

        if bud.is_windows and self._generator.startswith("gmake"):
            bud.add_to_path_with_binaries(bud.default_mingw_path)

        is_visual_studio = bud.is_windows and self._generator.startswith("vs")
        premake_to_cmake_vs = (
            bud.premake_to_cmake_gen[self._generator]
            if is_visual_studio
            else "MinGW Makefiles"
        )

        bud.add_to_path_with_binaries(bud.default_cmake_path)
        subprocess.run(
            [
                "cmake",
                ("-GUnix Makefiles" if bud.is_macos else f"-G{premake_to_cmake_vs}"),
                "-S",
                sfml_path,
                "-B",
                build_sfml_path,
                "-DCMAKE_BUILD_TYPE=Release",
                f"-DCMAKE_OSX_ARCHITECTURES={bud.os_architecture}",
                f"-DCMAKE_OSX_DEPLOYMENT_TARGET={bud.os_version.split('.')[0]}",
                "-DWARNINGS_AS_ERRORS=FALSE",
                f"-DBUILD_SHARED_LIBS={'TRUE' if bud.is_macos else 'FALSE'}",
            ],
            check=True,
        )
        subprocess.run(
            ["cmake", "--build", build_sfml_path, "--config", "Release"],
            check=True,
        )
        if is_visual_studio:
            subprocess.run(
                ["cmake", "--build", build_sfml_path, "--config", "Debug"],
                check=True,
            )
        print("Done.\n")

    def clean(self) -> None:
        print("Removing build files for SFML...")

        bud = Buddy()
        build_sfml_path = f"{bud.root_path}/vendor/SFML/build-sfml"
        try:
            shutil.rmtree(build_sfml_path)
            print(f"Removed {build_sfml_path}")
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
