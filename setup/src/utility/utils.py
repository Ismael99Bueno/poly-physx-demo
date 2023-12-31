from pathlib import Path


def download_file(url: str, path: Path) -> None:
    from tqdm import tqdm
    import requests

    path.parent.mkdir(exist_ok=True, parents=True)
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


def unzip_file(zip_path: Path, extract_path: Path) -> None:
    from tqdm import tqdm
    from zipfile import ZipFile

    extract_path.mkdir(exist_ok=True)
    with ZipFile(zip_path, "r") as zip:
        for file in tqdm(iterable=zip.namelist(), total=len(zip.namelist())):
            zip.extract(member=file, path=extract_path)

    zip_path.unlink()
