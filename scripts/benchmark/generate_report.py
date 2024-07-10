from pathlib import Path
from typing import Any
from time import perf_counter
from collections.abc import Sequence

from arguments import create_and_parse_generate_report_args
from report import generate_report, generate_combined_report

import pandas as pd
import multiprocess as mp
import yaml
import functools


def main(args: Sequence[str] | None = None, *, verbose: bool = True) -> None:
    t1 = perf_counter()
    args = create_and_parse_generate_report_args(args)
    input_paths = args.input

    output_path: Path = args.output
    output_path.mkdir(parents=True, exist_ok=True)

    if not output_path.is_dir():
        raise NotADirectoryError(f"Output path '{output_path}' is not a directory")

    csvs: dict[Path, pd.DataFrame] = {}
    summaries: dict[Path, Any] = {}
    for input_path in input_paths:
        input_path: Path
        if not input_path.is_dir():
            raise NotADirectoryError(f"Input path '{input_path}' is not a directory")

        for f in input_path.rglob("*"):
            if f.is_dir() or f.name.startswith("."):
                continue

            if args.combine:
                key = f.parent
            else:
                key = output_path / f.relative_to(input_path).parent

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
        fn = functools.partial(generate_report, csvs=csvs, args=args)
        with mp.Pool(args.jobs) as pool:
            pool.starmap(fn, summaries.items())
    else:
        for output_folder, summary in summaries.items():
            generate_report((output_folder, summary), csvs)

    if verbose:
        print(f"Done in {perf_counter() - t1:.2f} seconds")


if __name__ == "__main__":
    main()
