from setup_py import validate_python_version, validate_python_package
from setup_win_premake import validate_premake
from setup_win_cmake import validate_cmake
from exceptions import BadOS
import platform
import subprocess
from utils import ROOT_PATH


def main() -> None:
    if platform.system() != "Windows":
        raise BadOS(platform.system(), "Windows")

    validate_python_version()
    validate_python_package("requests")

    validate_premake()
    validate_cmake()

    while True:
        answer = input(
            "Do you want to go ahead and generate all the project build files? Default premake action for Windows is vs2022 [Y]/N "
        )
        if answer == "n" or answer == "N":
            print(
                "Run generate_build_files.py to generate the project's build files. Use the -h flag to display the options available"
            )
            break
        elif answer == "y" or answer == "Y" or answer == "":
            subprocess.run(
                [
                    "python",
                    f"{ROOT_PATH}/scripts/generate_build_files.py",
                    "--premake-action",
                    "vs2022",
                ]
            )
            break
    print("Setup completed successfully! (hopefully)")


if __name__ == "__main__":
    main()
