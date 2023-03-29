from argparse import ArgumentParser
import os
from utils import build_all, build_ppx, build_sfml, clean_all, clean_ppx, clean_sfml


def main() -> None:
    parser = ArgumentParser(description="Generate the build files of the project.")
    parser.add_argument("src", metavar="root-relpath", type=str)
    parser.add_argument("--which", dest="which", default="all")
    parser.add_argument(
        "--clean", dest="clean", action="store_const", const=True, default=False
    )

    args = parser.parse_args()

    root_path = os.path.abspath(args.src)
    if not os.path.exists(root_path):
        raise ValueError(f"Path {root_path} does not exist.")

    print(f"Setup wrt root: {root_path}\n")
    options = (
        {"all": clean_all, "ppx": clean_ppx, "sfml": clean_sfml}
        if args.clean
        else {"all": build_all, "ppx": build_ppx, "sfml": build_sfml}
    )
    try:
        options[args.which](root_path)
    except KeyError:
        raise ValueError(
            f"Unrecognized --which argument '{args.which}'. Expected 'all', 'ppx' or 'sfml'."
        )


if __name__ == "__main__":
    main()
