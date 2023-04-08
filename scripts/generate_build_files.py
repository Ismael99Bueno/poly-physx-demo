from setup_py import validate_python_version

validate_python_version()

from argparse import ArgumentParser
from generator import FullGenerator, PPXGenerator, SFMLGenerator, Generator
from exceptions import UnrecognizedWhichArgumentError
from utils import ROOT_PATH


def main() -> None:
    parser = ArgumentParser(
        description="Generate the build files for the project.",
        epilog="Once the build files have been generated, the project must be compiled according to the chosen premake5 action.",
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
        "--premake-action",
        dest="action",
        default="gmake2",
        type=str,
        help="Can be one of the actions listed in 'premake5 --help' option. Defaults to 'gmake2'",
    )

    args = parser.parse_args()

    print(f"Setup wrt root: {ROOT_PATH}\n")
    options = {
        "all": FullGenerator(args.action),
        "ppx": PPXGenerator(args.action),
        "sfml": SFMLGenerator(),
    }

    try:
        gen: Generator = options[args.which]
        if args.clean:
            gen.clean()
        else:
            gen.build()
            print(
                "\nCompile the project according to the selected premake action.\nIf you have selected gmake2 (default), use 'make' from the root folder to compile the project. Enter 'make help' to see all possible configurations. Choose the one compatible with your architecture.\nThe executable will be located in the poly-physx-demo subfolder's binaries.\nThe program should be executed from the source path for the executable to be able to locate the SFML shared libs."
            )
    except KeyError:
        raise UnrecognizedWhichArgumentError(args.which)


if __name__ == "__main__":
    main()
