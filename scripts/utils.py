from __future__ import annotations
import os
from pathlib import Path
from exceptions import PathNotFoundError
import platform
import time
import sys


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
    def mingw_path(self) -> str:
        return self.__mingw_path

    @mingw_path.setter
    def mingw_path(self, mingw_path: str) -> None:
        self.__mingw_path = mingw_path

    @property
    def cmake_path(self) -> str:
        return self.__cmake_path

    @cmake_path.setter
    def cmake_path(self, cmake_path: str) -> None:
        self.__cmake_path = cmake_path

    def add_to_path_with_binaries(self, path: str) -> None:
        os.environ["PATH"] += f"{os.pathsep}{path}"
        os.environ["PATH"] += f"{os.pathsep}{path}/bin"

    def prompt(self, message: str) -> bool:
        while True:
            answer = input(f"{message} [Y]/N ")
            if answer == "n" or answer == "N":
                return False
            elif answer == "y" or answer == "Y" or answer == "":
                return True

    def prompt_to_install(self, to_be_installed: str) -> bool:
        return self.prompt(
            f"{to_be_installed} installation not found. Do you wish to install?"
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
        self.__cmake_path = "C:/Program Files/CMake"
        self.__mingw_path = f"C:/mingw{'32' if self.os_architecture == 'x86' else '64'}"

        return self


def download_file(url: str, path: str) -> None:
    from tqdm import tqdm
    import requests

    os.makedirs(os.path.dirname(path), exist_ok=True)

    headers = {
        "User-Agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_4) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.97 Safari/537.36"
    }
    response = requests.get(url, headers=headers, stream=True, allow_redirects=True)
    total = int(response.headers.get("content-length", "0"))
    one_kb = 1024

    with open(path, "wb") as f, tqdm(
        total=total, unit="MiB", unit_scale=True, unit_divisor=one_kb * one_kb
    ) as bar:
        for data in response.iter_content(chunk_size=one_kb):
            size = f.write(data)
            bar.update(size)


def unzip_file(zip_path: str, extract_path: str) -> None:
    from tqdm import tqdm
    from zipfile import ZipFile

    os.makedirs(extract_path, exist_ok=True)

    with ZipFile(zip_path, "r") as zip:
        for file in tqdm(iterable=zip.namelist(), total=len(zip.namelist())):
            zip.extract(member=file, path=f"{extract_path}/{file}")

    os.remove(zip_path)
