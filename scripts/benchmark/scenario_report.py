from pathlib import Path
from time import perf_counter
from arguments import create_and_parse_scenario_report_args

import generate_report
import sys


def main() -> None:
    t1 = perf_counter()

    args = create_and_parse_scenario_report_args()
    common_args = []
    for i, arg in enumerate(sys.argv):
        if (
            i == 0
            or arg == "-n"
            or arg == "--name"
            or sys.argv[i - 1] == "-n"
            or sys.argv[i - 1] == "--name"
        ):
            continue
        common_args.append(arg)

    input_path = Path(f"output/benchmark/data/{args.name}")
    output_path: Path = args.output / args.name

    # standalone report generation
    genargs = common_args.copy()

    genargs.extend(["-i", str(input_path), "-o", str(output_path)])
    generate_report.main(genargs, verbose=False)

    # combined report between cycles of the same scenario
    common_args.append("-c")
    common_cycles: dict[Path, list[Path]] = {}
    for folder in input_path.glob("*"):
        if folder.is_file():
            continue

        genargs = common_args.copy()
        genargs.extend(["-i", str(folder), "-o", str(output_path / folder.name)])
        generate_report.main(genargs, verbose=False)

        crun_name = Path(folder.name.rsplit("-", 1)[0].removesuffix("-id"))
        for cycle in folder.glob("*"):
            if cycle.is_file():
                continue

            ccycle_relpath = crun_name / cycle.name
            if ccycle_relpath not in common_cycles:
                common_cycles[ccycle_relpath] = [cycle]
            else:
                common_cycles[ccycle_relpath].append(cycle)

    for ccycle_relpath, folders in common_cycles.items():
        genargs = common_args.copy()
        genargs.extend(
            [
                "-i",
                *[str(folder) for folder in folders],
                "-o",
                str(output_path / ccycle_relpath),
            ]
        )
        generate_report.main(genargs, verbose=False)

    print(f"Done in {perf_counter() - t1:.2f} seconds")


if __name__ == "__main__":
    main()
