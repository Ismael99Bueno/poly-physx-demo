from pathlib import Path
from argparse import ArgumentParser, Namespace
from collections.abc import Sequence


def create_and_parse_generate_report_args(
    args: Sequence[str] | None = None,
) -> Namespace:
    parser = ArgumentParser(
        description="Generate benchmark reports from performance data"
    )
    parser.add_argument(
        "-i",
        "--input",
        help="Path(s) where the data to process is. Default is 'output/benchmark/data'",
        default=[Path("output/benchmark/data")],
        type=Path,
        nargs="+",
    )
    parser.add_argument(
        "-c",
        "--combine",
        action="store_true",
        default=False,
        help="Combine all reports into a single file",
    )
    __add_common_arguments(parser)

    return parser.parse_known_args(args)[0]


def create_and_parse_scenario_report_args() -> Namespace:
    parser = ArgumentParser(
        description="Generate benchmark reports from performance data coming from known scenarios",
        epilog="The output will be saved in the 'output/benchmark/reports' folder",
    )

    parser.add_argument("--name", required=True, help="Name of the scenario", type=str)
    parser.add_argument(
        "--scenario-runs",
        type=str,
        default=None,
        help="Scenario runs to consider. Default is all of them",
    )
    parser.add_argument(
        "--scenario-groups",
        nargs="+",
        type=str,
        default=None,
        help="Groups of scenario runs to combine",
    )
    parser.add_argument(
        "--cycle-groups",
        nargs="+",
        help="Groups of scenario cycles to combine",
        type=str,
        default=None,
    )
    parser.add_argument(
        "-s",
        "--standalone",
        action="store_true",
        default=False,
        help="Generate standalone reports for each of the scenario cycles",
    )
    parser.add_argument(
        "-l",
        "--logarithmic",
        action="store_true",
        default=False,
        help="Use logarithmic scale",
    )

    __add_common_arguments(parser)
    return parser.parse_args()


def __add_common_arguments(parser: ArgumentParser) -> None:
    parser.add_argument(
        "-j",
        "--jobs",
        type=int,
        default=None,
        help="Number of jobs to run in parallel. Default is single threaded",
    )
    parser.add_argument(
        "-r",
        "--regenerate",
        action="store_true",
        help="Remove and regenerate all reports the input covers",
        default=False,
    )
    parser.add_argument(
        "-w",
        "--overwrite",
        action="store_true",
        help="Overwrite existing report files",
        default=False,
    )
    parser.add_argument(
        "-o",
        "--output",
        help="Output folder. Default is 'output/benchmark/reports'",
        default=Path("output/benchmark/reports"),
        type=Path,
    )
