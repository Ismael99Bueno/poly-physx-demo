from setup_py import validate_python_version, validate_python_package
from setup_win_premake import validate_premake


def main() -> None:
    validate_python_version()
    validate_python_package("requests")
    validate_premake()


if __name__ == "__main__":
    main()
