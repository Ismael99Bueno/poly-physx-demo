import os
from pathlib import Path
from exceptions import PathNotFoundError

ROOT_PATH = Path(os.path.dirname(os.path.realpath(__file__))).parent.absolute()
if not os.path.exists(ROOT_PATH):
    raise PathNotFoundError(ROOT_PATH)

PREMAKE_TO_CMAKE_GEN = {
    f"vs20{year}": f"Visual Studio {version} 20{year}"
    for year, version in zip(
        ["08", "10", "12", "13", "15", "17", "19", "22"],
        [9, 10, 11, 12, 14, 15, 16, 17],
    )
}


def download_file(url: str, path: str) -> None:
    import requests

    os.makedirs(os.path.dirname(path), exist_ok=True)

    headers = {
        "User-Agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_4) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.97 Safari/537.36"
    }
    response = requests.get(url, headers=headers, stream=True)
    with open(path, "wb") as f:
        f.write(response.content)


def unzip_file(zip_path: str, extract_path: str) -> None:
    from zipfile import ZipFile

    with ZipFile(zip_path, "r") as zip:
        zip.extractall(extract_path)

    os.remove(zip_path)
