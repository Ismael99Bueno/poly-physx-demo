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
    validate_python_packages(["requests", "tqdm"])
    validate_premake()
    validate_vulkan()
    if sys.argv[2].startswith("gmake"):
        validate_mingw()

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

    print("\nSetup completed successfully!")


def __compile_lynx_shaders() -> None:
    print("\nCompiling lynx shaders...")
    sys.path.append(Buddy().root_path)
    from lynx.scripts.win_compile_shaders import main

    main()


if __name__ == "__main__":
    main()
