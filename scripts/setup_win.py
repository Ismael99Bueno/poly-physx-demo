from setup_py import validate_python_version, validate_python_package
from setup_win_premake import validate_premake
from exceptions import BadOS
import platform


def main() -> None:
    if platform.system() != "Windows":
        raise BadOS(platform.system(), "Windows")

    validate_python_version()
    validate_python_package("requests")
    validate_premake()


if __name__ == "__main__":
    main()
