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
        is_all = self._generator == "all"
        is_gmake = is_all or self._generator.startswith("gmake")
        is_vs = is_all or self._generator.startswith("vs")
        print("Removing build files for PPX...")

        bud = Buddy()
        for folder in [".", "vendor"]:
            if is_gmake:
                PPXGenerator.__remove(f"{bud.root_path}/{folder}/*/Makefile", os.remove)
            if is_vs:
                PPXGenerator.__remove(
                    f"{bud.root_path}/{folder}/*/*.vcxproj*", os.remove
                )
                PPXGenerator.__remove(f"{bud.root_path}/{folder}/*/*.sln", os.remove)
        PPXGenerator.__remove(f"{bud.root_path}/*.sln", os.remove)
        if is_gmake and os.path.exists(f"{bud.root_path}/Makefile"):
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
        self.__build_name = "vs" if self._generator.startswith("vs") else "gmake"

    def build(self) -> None:
        bud = Buddy()

        print("Generating build files for SFML...")
        build_path = f"{bud.root_path}/vendor/SFML/build-{self.__build_name}"

        if os.path.exists(build_path):
            print(
                f"Skipping, build path {build_path} already exists. Run --clean option to re-build\n"
            )
            return

        sfml_path = f"{bud.root_path}/vendor/SFML"
        if not os.path.exists(sfml_path):
            raise PathNotFoundError(sfml_path)

        os.mkdir(build_path)

        if bud.is_windows and self._generator.startswith("gmake"):
            bud.add_to_path_with_binaries(bud.mingw_path)

        is_visual_studio = bud.is_windows and self._generator.startswith("vs")
        premake_to_cmake_vs = (
            bud.premake_to_cmake_gen[self._generator]
            if is_visual_studio
            else "MinGW Makefiles"
        )

        bud.add_to_path_with_binaries(bud.cmake_path)
        subprocess.run(
            [
                "cmake",
                ("-GUnix Makefiles" if bud.is_macos else f"-G{premake_to_cmake_vs}"),
                "-S",
                sfml_path,
                "-B",
                build_path,
                "-DCMAKE_BUILD_TYPE=Release",
                f"-DCMAKE_OSX_ARCHITECTURES={bud.os_architecture}",
                f"-DCMAKE_OSX_DEPLOYMENT_TARGET={bud.os_version.split('.')[0]}",
                "-DWARNINGS_AS_ERRORS=FALSE",
                f"-DBUILD_SHARED_LIBS={'TRUE' if bud.is_macos else 'FALSE'}",
            ],
            check=True,
        )
        subprocess.run(
            ["cmake", "--build", build_path, "--config", "Release"],
            check=True,
        )
        if is_visual_studio:
            subprocess.run(
                ["cmake", "--build", build_path, "--config", "Debug"],
                check=True,
            )
        print("Done.\n")

    def clean(self) -> None:
        bud = Buddy()
        if self._generator == "all":
            self.__clean(f"{bud.root_path}/vendor/SFML/build-vs")
            self.__clean(f"{bud.root_path}/vendor/SFML/build-gmake")
        else:
            self.__clean(f"{bud.root_path}/vendor/SFML/build-{self.__build_name}")

    def __clean(self, build_sfml_path: str) -> None:
        print("Removing SFML build...")
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
