from __future__ import annotations
from pathlib import Path
from validation.exceptions import PathNotFoundError

import os
import platform


class Buddy:
    __instance = None

    def __new__(cls) -> Buddy:
        if cls.__instance is None:
            cls.__instance = super().__new__(cls).__init()
        return cls.__instance

    def __init(self) -> Buddy:
        self.__root_path = Path(
            os.path.dirname(os.path.realpath(__file__))
        ).parent.parent.parent.absolute()

        if not self.__root_path.exists():
            raise PathNotFoundError(f"Root path {self.__root_path} was not found")

        self.__windows_mingw_path = Path(
            "C:", "mingw", "32" if self.os_architecture == "x86" else "64"
        )
        self.__accept_all_prompts = False
        return self

    def add_to_premake_path(self, path: str) -> None:
        if "PREMAKE_PATH" in os.environ:
            os.environ["PREMAKE_PATH"] += f";{path}"
        else:
            os.environ["PREMAKE_PATH"] = path.__str__()

    def add_to_path_with_binaries(self, path: str) -> None:
        os.environ["PATH"] += f"{os.pathsep}{path}"
        os.environ["PATH"] += f"{os.pathsep}{path}/bin"

    def prompt(self, message: str, default: bool = True) -> bool:
        if self.__accept_all_prompts:
            return True
        while True:
            answer = input(f"{message} [Y]/N " if default else f"{message} Y/[N] ")
            if answer == "n" or answer == "N" or (not default and answer == ""):
                return False
            elif answer == "y" or answer == "Y" or (default and answer == ""):
                return True

    def prompt_to_install(self, to_be_installed: str) -> bool:
        return self.prompt(
            f"{to_be_installed} installation not found. Do you wish to install?"
        )

    def accept_all_prompts(self) -> None:
        self.__accept_all_prompts = True

    @property
    def root_path(self) -> Path:
        return self.__root_path

    @property
    def current_os(self) -> str:
        return platform.system()

    @property
    def is_macos(self) -> bool:
        return platform.system() == "Darwin"

    @property
    def is_windows(self) -> bool:
        return platform.system() == "Windows"

    @property
    def is_os_unsupported(self) -> bool:
        return not self.is_macos and not self.is_windows

    @property
    def os_version(self) -> str:
        if self.is_macos:
            return platform.mac_ver()[0]
        return platform.version()

    @property
    def os_architecture(self) -> str:
        similars = {"i386": "x86", "amd64": "x86_64", "x32": "x86", "x64": "x86_64"}
        try:
            return similars[platform.machine().lower()]
        except KeyError:
            return platform.machine().lower()

    @property
    def windows_mingw_path(self) -> str:
        return self.__windows_mingw_path

    @windows_mingw_path.setter
    def windows_mingw_path(self, windows_mingw_path: Path) -> None:
        self.__windows_mingw_path = windows_mingw_path
