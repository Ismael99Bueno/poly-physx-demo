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
        description="Compile the SFML library and generate the build files for the project.",
        epilog="Once the build files have been generated, the project must be compiled according to the chosen generator and your operating system. Instructions will be given once the execution of this script has been successful",
    )
    parser.add_argument(
        "-w",
        "--which",
        dest="which",
        default="all",
        type=str,
        help="can be one of the following: 'sfml' - builds SFML as a shared (MacOS) or static (Windows) library using CMake. 'ppx' -  generates poly-physx-demo's build files with premake5. SFML must be built first. 'all' - executes both 'sfml' and 'ppx'. Default: 'all'. If the '--clean' option is enabled, it will erase the file generated for the corresponding selection",
    )
    parser.add_argument(
        "-c",
        "--clean",
        dest="clean",
        action="store_const",
        const=True,
        default=False,
        help="clears all build files for the selected project component and generator, specified with '--which', and '--generator'. If all generators are to be cleaned, you can specify '--generator all'",
    )
    parser.add_argument(
        "-g",
        "--generator",
        dest="generator",
        default="gmake2" if bud.is_macos else "vs2022",
        type=str,
        help="Can be 'gmake' or 'gmake2'. The latter is recommended as 'gmake' is deprecated. If on Windows, it can also be any of the premake actions for Visual Studio. If the '--clean' option is enabled, 'all' can be passed as well to clean all generators",
    )
    parser.add_argument(
        "-y",
        "--yes",
        dest="all_yes",
        action="store_const",
        const=True,
        default=False,
        help="confirms all installations by default",
    )

    if bud.is_os_unsupported:
        raise BadOSError(bud.current_os, "MacOS or Windows")

    args = parser.parse_args()
    bud.all_yes = args.all_yes

    if (
        not args.generator.startswith("vs")
        and not args.generator.startswith("gmake")
        and (args.generator != "all" or not args.clean)
    ) or (args.generator.startswith("vs") and bud.is_macos):
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
            print(
                f"\nCleaned build files for {args.which} for {args.generator} generator(s)"
            )
        else:
            gen.build()
            print(
                "\nBuild files have been successfully generated. Next step: build the project"
            )
            if bud.is_macos:
                print("\n==== MacOS instructions ====")
                print(
                    f"Build the project from terminal with make by entering 'make' from the root directory at {bud.root_path}"
                )
            elif bud.is_windows and args.generator.startswith("vs"):
                print("\n==== Windows instructions (Visual Studio) ====")
                print(
                    f"Open the solution file at {bud.root_path}/poly-physx-demo.sln and build the project with Visual Studio, which must be installed for this configuration to work"
                )
                print(
                    "If you don't want to install Visual Studio, you can build the project with the gmake2 generator to generate MinGW makefiles. MinGW will be automatically installed"
                )
            elif bud.is_windows and args.generator.startswith("gmake"):
                print("\n==== Windows instructions (MinGW) ====")
                print(
                    f"ATTENTION: To avoid any issues, make sure to add the MinGW folder and binaries to path by running 'set PATH=%PATH%;{bud.mingw_path}/bin' in the current cmd session. You should also run the executable from this session to avoid having to copy the required dlls. If you do need them because the executable asks for them, you will find them at {bud.mingw_path}/bin folder."
                )
                print(
                    f"\nBuild the project from terminal with make by entering 'mingw32-make' from the root directory at {bud.root_path}"
                )

            print(
                f"Once the compilation finishes, execute the binary from the 'bin' folder at the poly-physx-demo project, at {bud.root_path}/poly-physx-demo/bin"
            )
    except KeyError:
        raise UnrecognizedWhichArgumentError(args.which)


if __name__ == "__main__":
    main()
