from setup_py import validate_python_version

validate_python_version()

from argparse import ArgumentParser
from generator import FullGenerator, PPXGenerator, SFMLGenerator, Generator
from exceptions import (
    UnrecognizedWhichArgumentError,
    GeneratorNotSupportedError,
    BadOSError,
)
from utils import Buddy
from setup_win_mingw import validate_mingw


def main() -> None:
    bud = Buddy()
    parser = ArgumentParser(
        description="Generate the build files for the project.",
        epilog="Once the build files have been generated, the project must be compiled according to the chosen generator.",
    )
    parser.add_argument(
        "--which",
        dest="which",
        default="all",
        type=str,
        help="can be one of the following: 'sfml' - builds SFML as a shared library using CMake, 'ppx' -  generates poly-physx-demo's build files with premake5. SFML must be built first or 'all' - executes both 'sfml' and 'ppx'. Default: 'all'",
    )
    parser.add_argument(
        "--clean",
        dest="clean",
        action="store_const",
        const=True,
        default=False,
        help="clears all build files for the selected project component",
    )
    parser.add_argument(
        "--generator",
        dest="generator",
        default="gmake2" if bud.is_macos else "vs2022",
        type=str,
        help="Can be gmake or gmake2. If on windows, it can be any of the premake actions for Visual Studio.",
    )

    if bud.is_os_unsupported:
        raise BadOSError("MacOS or Windows", bud.current_os)

    args = parser.parse_args()
    if (
        not args.generator.startswith("vs")
        and not args.generator.startswith("gmake")
        or args.generator.startswith("vs")
        and bud.is_macos
    ):
        raise GeneratorNotSupportedError(args.generator)

    if args.generator.startswith("gmake") and bud.is_windows:
        validate_mingw()

    print(f"Setup wrt root: {bud.root_path}\n")
    options = {
        "all": FullGenerator(args.generator),
        "ppx": PPXGenerator(args.generator),
        "sfml": SFMLGenerator(args.generator),
    }

    try:
        gen: Generator = options[args.which]
        if args.clean:
            gen.clean()
        else:
            gen.build()
            print("\nCompile the project according to the selected generator.\n")
            print(
                "If you have selected gmake/gmake2, use 'make' from the root folder to compile the project. Enter 'make help' to see all possible configurations. Choose the one compatible with your architecture."
            )
            print(
                "If you have selected Visual Studio, open the solution and build it from there, choosing a configuration and architecture."
            )
            print(
                "\nThe executable will be located in the poly-physx-demo subfolder's binaries."
            )
    except KeyError:
        raise UnrecognizedWhichArgumentError(args.which)


if __name__ == "__main__":
    main()
