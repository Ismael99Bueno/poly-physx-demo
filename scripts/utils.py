import os
from pathlib import Path
from exceptions import PathNotFoundError

ROOT_PATH = Path(os.path.dirname(os.path.realpath(__file__))).parent.absolute()
if not os.path.exists(ROOT_PATH):
    raise PathNotFoundError(ROOT_PATH)


def download_file(url: str, path: str) -> None:
    import requests

    os.makedirs(path, exist_ok=True)

    headers = {
        "User-Agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_4) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.97 Safari/537.36"
    }
    response = requests.get(url, headers=headers, stream=True)
    with open(path, "wb") as f:
        f.write(response.content)


def unzip_file(path: str) -> None:
    from zipfile import ZipFile

    with ZipFile(path, "r") as zip:
        zip.extractall()

    os.remove(path)
