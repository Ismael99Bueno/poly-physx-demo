from validation.python_validation import validate_python_version

validate_python_version()

from pathlib import Path
from utility.arguments import build_and_retrieve_arguments
from utility.buddy import Buddy

import subprocess


def build(generator: str) -> None:
    print("\n==== BUILD FILES GENERATION ====")
    bud = Buddy()

    export_compile_commands_path = bud.root_path / Path(
        "vendor", "premake-export-compile-commands"
    )
    premake_windows_executable_path = bud.root_path / Path(
        "vendor", "premake", "bin", "premake5.exe"
    )
    premake_file_path = bud.root_path / Path("premake5.lua")

    bud.add_to_premake_path(export_compile_commands_path)
    subprocess.run(
        [
            "premake5" if bud.is_macos else premake_windows_executable_path.__str__(),
            f"--file={premake_file_path}",
            generator,
        ],
        shell=bud.is_windows,
        check=True,
    )


def clean(generator: str) -> None:
    print("\n==== BUILD FILES REMOVAL ====")

    is_gmake = generator.startswith("gmake")
    is_vs = generator.startswith("vs")

    bud = Buddy()
    vendor_folder = bud.root_path / Path("vendor")
    for folder in [bud.root_path, vendor_folder]:
        if is_gmake:
            for makefile in folder.glob("*/Makefile"):
                print(f"Removing {makefile.absolute()}")
                makefile.unlink()
        elif is_vs:
            for vcxproj in folder.glob("**.vcxproj*"):
                print(f"Removing {vcxproj.absolute()}")
                vcxproj.unlink()
            for sln in folder.glob("**.sln"):
                print(f"Removing {sln.absolute()}")
                sln.unlink()
    print("Done")


def main() -> None:
    generator, cleanse = build_and_retrieve_arguments()
    if cleanse:
        clean(generator)
    else:
        build(generator)


if __name__ == "__main__":
    main()
