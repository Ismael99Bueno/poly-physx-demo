import os
from pathlib import Path
from exceptions import PathNotFoundError

ROOT_PATH = Path(os.path.dirname(os.path.realpath(__file__))).parent.absolute()
if not os.path.exists(ROOT_PATH):
    raise PathNotFoundError(ROOT_PATH)
