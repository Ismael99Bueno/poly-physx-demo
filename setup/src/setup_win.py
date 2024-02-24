from validation.python_validation import (
    validate_python_version,
    validate_python_packages,
)
from validation.premake_validation import validate_premake
from validation.vulkan_validation import validate_vulkan
from validation.mingw_validation import validate_mingw


validate_python_version()

from utility.buddy import Buddy
from validation.exceptions import BadOSError

import sys
import subprocess


def main() -> None:
    bud = Buddy()
    if not bud.is_windows:
        raise BadOSError(
            f"This setup can only be run if the current OS is windows, but the current OS is '{bud.current_os}'"
        )
    if validate_python_packages(["requests", "tqdm"]):
        print(
            "Python packages had to be installed. Re run this script for the changes to take effect"
        )
        quit()
    validate_premake()
    validate_vulkan()
    if sys.argv[2].startswith("gmake"):
        validate_mingw()
        print("Attempting to add MinGW binaries to path...")
        subprocess.run(
            ["set", f"PATH=%PATH%;{bud.windows_mingw_path}"], shell=True, check=True
        )

    __compile_lynx_shaders()

    subprocess.run(
        [
            "python",
            f"{bud.root_path}/setup/src/generate_build_files.py",
            sys.argv[1],
            sys.argv[2],
        ],
        check=True,
    )
    print(
        "Run generate_build_files.py with python to have further detail on how the project can be built. Use the -h flag to display the options available"
    )

    print(
        "\nIf you are on a laptop and vulkan is unable to find a suitable physical device, try setting the DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1 environment variable to 1"
    )
    print(
        f"If running with gmake generator, make sure to run 'mingw32-make' instead of 'make' to build the project. If the command fails, make sure to have MinGW binaries in your path by running 'set PATH=%PATH%;{bud.windows_mingw_path}' in the command prompt before running the build command"
    )
    print("\nSetup completed successfully!")


def __compile_lynx_shaders() -> None:
    print("\nCompiling lynx shaders...")
    sys.path.append(str(Buddy().root_path))
    from lynx.scripts.win_compile_shaders import main

    main()


if __name__ == "__main__":
    main()
