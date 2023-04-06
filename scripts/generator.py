import os
import subprocess
import shutil
import glob
from typing import Callable
from abc import ABC, abstractmethod
import platform


class Generator(ABC):
    def __init__(self, root_path: str) -> None:
        self._root_path = root_path

    @abstractmethod
    def build(self) -> None:
        ...

    @abstractmethod
    def clean(self) -> None:
        ...


class PPXGenerator(Generator):
    def __init__(self, root_path: str) -> None:
        super().__init__(root_path)

    def build(self) -> None:
        print("Generating build files for poly-physx...")
        subprocess.run(["premake5", f"--file={self._root_path}/premake5.lua", "gmake2"])

    def clean(self) -> None:
        print("Removing build files for poly-physx...")
        PPXGenerator.__remove(f"{self._root_path}/*/bin")
        PPXGenerator.__remove(f"{self._root_path}/vendor/*/bin")
        PPXGenerator.__remove(f"{self._root_path}/*/build")
        PPXGenerator.__remove(f"{self._root_path}/vendor/*/build")
        PPXGenerator.__remove(f"{self._root_path}/*/Makefile", os.remove)
        PPXGenerator.__remove(f"{self._root_path}/vendor/*/Makefile", os.remove)
        if os.path.exists(f"{self._root_path}/Makefile"):
            os.remove(f"{self._root_path}/Makefile")

        print("Done.\n")

    @staticmethod
    def __remove(pattern: str, remfunc: Callable[[str], None] = shutil.rmtree) -> None:
        for file in glob.glob(pattern):
            print(f"Removing {file}...")
            remfunc(file)


class SFMLGenerator(Generator):
    def __init__(self, root_path: str) -> None:
        super().__init__(root_path)

    def build(self) -> None:
        self.clean()
        print("Generating build files for SFML...")

        sfml_path = f"{self._root_path}/vendor/SFML"
        if not os.path.exists(sfml_path):
            raise FileNotFoundError(
                f"{sfml_path} not found. Did you pass the source path correctly?"
            )
        build_sfml_path = f"{self._root_path}/vendor/SFML/build-sfml"
        os.mkdir(build_sfml_path)

        mac_ver, _, arch = platform.mac_ver()
        subprocess.run(
            [
                "cmake",
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
        build_sfml_path = f"{self._root_path}/vendor/SFML/build-sfml"
        try:
            shutil.rmtree(build_sfml_path)
        except FileNotFoundError:
            print(
                f"Attempted to remove {build_sfml_path} directory, but it does not exist. Skipping..."
            )
        print("Done.\n")


class FullGenerator(Generator):
    def __init__(self, root_path: str) -> None:
        super().__init__(root_path)
        self.__ppx_gen = PPXGenerator(root_path)
        self.__sfml_gen = SFMLGenerator(root_path)

    def build(self) -> None:
        self.__sfml_gen.build()
        self.__ppx_gen.build()

    def clean(self) -> None:
        self.__ppx_gen.clean()
        self.__sfml_gen.clean()
