from argparse import ArgumentParser
import os
from generator import FullGenerator, PPXGenerator, SFMLGenerator, Generator


def main() -> None:
    parser = ArgumentParser(
        description="Generate the build files for the project.",
        epilog="Once the build files have been generated, the project must be compiled according to the chosen premake5 action.",
    )
    parser.add_argument(
        "src",
        metavar="root-relpath",
        type=str,
        help="the relative path to the project root",
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

    root_path = os.path.abspath(args.src)
    if not os.path.exists(root_path):
        raise ValueError(f"Path {root_path} does not exist.")

    print(f"Setup wrt root: {root_path}\n")
    options = {
        "all": FullGenerator(root_path),
        "ppx": PPXGenerator(root_path, args.action),
        "sfml": SFMLGenerator(root_path),
    }

    try:
        gen: Generator = options[args.which]
        if args.clean:
            gen.clean()
        else:
            gen.build()
            print(
                "Use 'make' to compile the project. Enter 'make help' to see all possible configurations. Choose the one compatible with your architecture.\nThe executable will be located in the poly-physx-demo subfolder's binaries."
            )
    except KeyError:
        raise ValueError(
            f"Unrecognized --which argument '{args.which}'. Expected 'all', 'ppx' or 'sfml'."
        )


if __name__ == "__main__":
    main()
