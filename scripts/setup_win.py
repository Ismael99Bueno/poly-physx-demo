from setup_py import validate_python_version, validate_python_package
from setup_win_premake import validate_premake
from setup_win_cmake import validate_cmake
from exceptions import BadOS
import platform


def main() -> None:
    if platform.system() != "Windows":
        raise BadOS(platform.system(), "Windows")

    validate_python_version()
    validate_python_package("requests")

    validate_premake()
    validate_cmake()

    print("Setup completed successfully!")


if __name__ == "__main__":
    main()
