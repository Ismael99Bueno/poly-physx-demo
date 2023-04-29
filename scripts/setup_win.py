from setup_py import validate_python_version, validate_python_packages
from setup_win_premake import validate_premake
from setup_win_cmake import validate_cmake
from exceptions import BadOSError
import subprocess
from utils import Buddy
import sys


def main() -> None:
    bud = Buddy()
    if not bud.is_windows:
        raise BadOSError("Windows", bud.current_os)

    validate_python_version()
    validate_python_packages(["requests", "tqdm"])

    validate_premake()
    validate_cmake()

    subprocess.run(
        [
            "python",
            f"{bud.root_path}/scripts/generate_build_files.py {sys.argv[1]} {sys.argv[2]}",
        ],
        check=True,
    )
    print(
        "Run generate_build_files.py with python to have further detain on how the project can be built. Use the -h flag to display the options available"
    )

    print("\nSetup completed successfully!")


if __name__ == "__main__":
    main()
