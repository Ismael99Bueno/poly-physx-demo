from setup_py import validate_python_version, validate_python_package
from setup_win_premake import validate_premake
from setup_win_cmake import validate_cmake
from exceptions import BadOSError
import subprocess
from utils import Buddy


def main() -> None:
    bud = Buddy()
    if not bud.is_windows:
        raise BadOSError("Windows", bud.current_os)

    validate_python_version()
    validate_python_package("requests")

    validate_premake()
    validate_cmake()

    while True:
        answer = input(
            "Do you want to go ahead and generate all the project build files? Default generator for Windows is vs2022 [Y]/N "
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
                    f"{bud.root_path}/scripts/generate_build_files.py",
                ],
                capture_output=True,
                check=True,
            )
            break
    print("Setup completed successfully! (hopefully)")


if __name__ == "__main__":
    main()
