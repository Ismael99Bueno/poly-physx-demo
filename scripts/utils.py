from __future__ import annotations
import os
from pathlib import Path
from exceptions import PathNotFoundError
import platform


class Buddy:
    __instance = None

    def __new__(cls) -> Buddy:
        if cls.__instance is None:
            cls.__instance = super().__new__(cls).__init()
        return cls.__instance

    @property
    def root_path(self) -> str:
        return self.__root_path

    @property
    def premake_to_cmake_gen(self) -> str:
        return self.__premake_to_cmake_gen

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
    def default_mingw_path(self) -> str:
        return self.__mingw_path

    @property
    def default_cmake_path(self) -> str:
        return self.__cmake_path

    def add_to_path_with_binaries(self, path: str) -> None:
        os.environ["PATH"] += f"{os.pathsep}{path}"
        os.environ["PATH"] += f"{os.pathsep}{path}\\bin"

    def prompt(self, message: str) -> bool:
        while True:
            answer = input(message)
            if answer == "n" or answer == "N":
                return False
            elif answer == "y" or answer == "Y" or answer == "":
                return True

    def prompt_to_install(self, to_be_installed: str) -> bool:
        return self.prompt(
            f"{to_be_installed} installation not found. Do you wish to install? [Y]/N "
        )

    def __init(self) -> Buddy:
        self.__root_path = Path(
            os.path.dirname(os.path.realpath(__file__))
        ).parent.absolute()

        if not os.path.exists(self.__root_path):
            raise PathNotFoundError(self.__root_path)

        self.__premake_to_cmake_gen = {
            f"vs20{year}": f"Visual Studio {version} 20{year}"
            for year, version in zip(
                ["08", "10", "12", "13", "15", "17", "19", "22"],
                [9, 10, 11, 12, 14, 15, 16, 17],
            )
        }
        self.__cmake_path = "C:\\Program Files\\CMake"
        self.__mingw_path = "C:\\MinGW"

        return self


def download_file(url: str, path: str, has_headers: bool = True) -> None:
    import requests

    os.makedirs(os.path.dirname(path), exist_ok=True)

    headers = (
        {
            "User-Agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_4) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.97 Safari/537.36"
        }
        if has_headers
        else None
    )
    response = requests.get(url, headers=headers, stream=True, allow_redirects=True)
    with open(path, "wb") as f:
        f.write(response.content)


def unzip_file(zip_path: str, extract_path: str) -> None:
    if zip_path.rsplit(".")[-1] == "7z":
        __unzip_7z(zip_path, extract_path)
    else:
        __unzip_builtin(zip_path, extract_path)


def __unzip_builtin(zip_path: str, extract_path: str) -> None:
    from zipfile import ZipFile

    with ZipFile(zip_path, "r") as zip:
        zip.extractall(extract_path)

    os.remove(zip_path)


def __unzip_7z(zip_path: str, extract_path: str) -> None:
    pass
    # from py7zr import SevenZipFile

    # with SevenZipFile(zip_path, "r") as sz:
    #     sz.extractall(extract_path)

    # os.remove(zip_path)
