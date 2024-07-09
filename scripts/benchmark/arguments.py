from pathlib import Path
from argparse import ArgumentParser, Namespace


def create_and_parse_args() -> Namespace:
    parser = ArgumentParser(
        description="Generate benchmark reports from performance data",
        epilog="The output will be saved in the 'output/benchmark-reports' folder",
    )
    parser.add_argument(
        "-j",
        "--jobs",
        dest="jobs",
        type=int,
        default=None,
        help="Number of jobs to run in parallel. Default is single threaded",
    )
    parser.add_argument(
        "-r",
        "--regenerate",
        action="store_true",
        help="Regenerate all reports",
        default=False,
    )
    parser.add_argument(
        "-i",
        "--input",
        dest="input",
        help="Path(s) where the data to process is. Default is 'output/benchmark-data'",
        default=[Path("output/benchmark/data")],
        type=Path,
        nargs="+",
    )
    parser.add_argument(
        "-o",
        "--output",
        dest="output",
        help="Output folder. Default is 'output/benchmark-reports'",
        default=Path("output/benchmark/reports"),
        type=Path,
    )
    parser.add_argument(
        "-c",
        "--combine",
        action="store_true",
        default=False,
        help="Combine all reports into a single ffile",
    )
    return parser.parse_args()
