from pathlib import Path
from typing import Any
from time import perf_counter
from collections.abc import Sequence

from arguments import create_and_parse_args
from report import generate_report, generate_combined_report

import pandas as pd
import multiprocess as mp
import yaml
import shutil
import functools
import re


def main(args: Sequence[str] | None = None) -> None:
    t1 = perf_counter()
    args = create_and_parse_args(args)
    if not args.regex:
        input_paths = args.input
    else:
        folder = Path("output/benchmark/data")
        regex = args.input
        input_paths = []
        for f in folder.rglob("*"):
            if f.is_dir() or f.name.startswith("."):
                continue
            if any([re.match(str(rx), f.parent.name) for rx in regex]):
                input_paths.append(f.parent)

        if len(input_paths) == 0:
            raise FileNotFoundError(
                f"No folders in '{folder}' match the regex '{args.regex}'"
            )

    output_path: Path = args.output.resolve()
    if args.combine:
        if output_path.exists() and args.regenerate:
            shutil.rmtree(output_path)
        elif output_path.exists():
            print(f"Done in {perf_counter() - t1:.2f} seconds")
            return

    output_path.mkdir(parents=True, exist_ok=True)
    if not output_path.is_dir():
        raise NotADirectoryError(f"Output path '{output_path}' is not a directory")

    csvs: dict[Path | str, pd.DataFrame] = {}
    summaries: dict[Path | str, Any] = {}
    for input_path in input_paths:
        input_path: Path = input_path.resolve()
        if not input_path.is_dir():
            raise NotADirectoryError(f"Input path '{input_path}' is not a directory")

        for f in input_path.rglob("*"):
            if f.is_dir() or f.name.startswith("."):
                continue

            if args.combine:
                key = f.parent.name
            else:
                key = output_path / f.relative_to(input_path).parent
                if key.exists() and args.regenerate:
                    shutil.rmtree(key)
                elif key.exists() and not args.combine:
                    continue
                key.mkdir(parents=True, exist_ok=True)

            if f.suffix == ".csv":
                data = pd.read_csv(f.absolute())
                csvs[key] = data
            else:
                with open(f, "r") as ffile:
                    data = yaml.safe_load(ffile)
                summaries[key] = data

    if args.combine:
        generate_combined_report(summaries, csvs, args)
    elif args.jobs is not None:
        fn = functools.partial(generate_report, csvs=csvs)
        with mp.Pool(args.jobs) as pool:
            pool.starmap(fn, summaries.items())
    else:
        for output_folder, summary in summaries.items():
            generate_report((output_folder, summary), csvs)

    print(f"Done in {perf_counter() - t1:.2f} seconds")


if __name__ == "__main__":
    main()
