from argparse import ArgumentParser
from utility.buddy import Buddy


def build_and_retrieve_arguments() -> tuple[str, bool]:
    bud = Buddy()

    parser = ArgumentParser(
        description="Generate the build files for the project.",
        epilog="Once the build files have been generated, the project must be compiled according to the chosen generator and your operating system. Instructions will be given once the execution of this script has been successful.",
    )

    parser.add_argument(
        "-g",
        "--generator",
        dest="generator",
        default="gmake2" if bud.is_macos else "vs2022",
        type=str,
        help="Can be 'gmake' or 'gmake2'. The latter is recommended as 'gmake' is deprecated. If on Windows, it can also be any of the premake actions for Visual Studio. If the '--clean' option is enabled, 'all' can be passed as well to clean all generators.",
    )

    parser.add_argument(
        "-c",
        "--clean",
        dest="clean",
        action="store_const",
        const=True,
        default=False,
        help="Clears all build files for the selected project component and generator, specified with '--which', and '--generator'. If all generators are to be cleaned, you can specify '--generator all'.",
    )

    parser.add_argument(
        "-y",
        "--yes",
        dest="all_prompts_accepted",
        action="store_const",
        const=True,
        default=False,
        help="Confirms all installations by default.",
    )

    args = parser.parse_args()
    if args.all_prompts_accepted:
        bud.accept_all_prompts()

    return args.generator, args.clean
