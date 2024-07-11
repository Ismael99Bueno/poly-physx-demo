from pathlib import Path
from time import perf_counter
from arguments import create_and_parse_scenario_report_args

import generate_report
import sys


def main() -> None:
    t1 = perf_counter()

    args = create_and_parse_scenario_report_args()
    common_args = sys.argv[1:]

    input_path = Path(f"output/benchmark/data/{args.name}")
    output_path: Path = args.output / args.name

    if args.groups is not None:
        groups: dict[str, list[int]] = {}
        for group in args.groups:
            groups[group] = [int(cycle) for cycle in group]

    # standalone report generation
    if args.standalone:
        genargs = common_args.copy()

        genargs.extend(["-i", str(input_path), "-o", str(output_path)])
        generate_report.main(genargs, verbose=False)

    common_args.append("-c")
    common_cycles: dict[Path, list[str]] = {}
    for folder in sorted(input_path.glob("*")):
        if folder.is_file():
            continue

        # full combined report between cycles
        if args.full_combined:
            genargs = common_args.copy()
            genargs.extend(["-i", str(folder), "-o", str(output_path / folder.name)])
            generate_report.main(genargs, verbose=False)

        grouped_cycle_paths: dict[str, list[str]] = {}

        crun_name = Path(folder.name.rsplit("-", 1)[0].removesuffix("-id"))
        for cycle in sorted(folder.glob("*")):
            if cycle.is_file():
                continue

            ccycle_relpath = crun_name / cycle.name
            if ccycle_relpath not in common_cycles:
                common_cycles[ccycle_relpath] = [str(cycle)]
            else:
                common_cycles[ccycle_relpath].append(str(cycle))
            if args.groups is None:
                continue

            cycle_id = int(cycle.name.split("-")[0])
            for group_str, group_ints in groups.items():
                if cycle_id not in group_ints:
                    continue
                if group_str not in grouped_cycle_paths:
                    grouped_cycle_paths[group_str] = [str(cycle)]
                else:
                    grouped_cycle_paths[group_str].append(str(cycle))

        # combined report for user defined groups
        for group_str, cycles in grouped_cycle_paths.items():
            genargs = common_args.copy()
            genargs.extend(
                [
                    "-i",
                    *cycles,
                    "-o",
                    str(output_path / f"{folder.name}-{group_str}"),
                ]
            )
            generate_report.main(genargs, verbose=False)

    # combined report between common cycles in different scenario runs
    if args.common_combined:
        for ccycle_relpath, folders in common_cycles.items():
            if len(folders) == 1:
                continue
            genargs = common_args.copy()
            genargs.extend(
                [
                    "-i",
                    *folders,
                    "-o",
                    str(output_path / ccycle_relpath),
                ]
            )
            generate_report.main(genargs, verbose=False)

    print(f"Done in {perf_counter() - t1:.2f} seconds")


if __name__ == "__main__":
    main()
