from pathlib import Path
from typing import Any
from time import perf_counter
from argparse import ArgumentParser, Namespace

import plotly.graph_objects as go
import plotly.express as px
import pandas as pd
import multiprocess as mp
import yaml
import markdown
import shutil
import functools


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
        default=[Path("output/benchmark-data")],
        type=Path,
        nargs="+",
    )
    parser.add_argument(
        "-o",
        "--output",
        dest="output",
        help="Output folder. Default is 'output/benchmark-reports'",
        default=Path("output/benchmark-reports"),
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


def write_markdown_recursive(summary: dict, content: str = "", depth: int = 1) -> str:
    for key, value in summary.items():
        if isinstance(value, dict):
            content += f"{'#' * depth} {key}\n"
            content = write_markdown_recursive(value, content, depth + 1)
        else:
            content += f"- {key}: {value}\n"
    return content


def create_plot_from_df(
    df: pd.DataFrame, color=None, name: str = None
) -> dict[str, go.Figure]:
    df["Time"] = df["Timestep"].cumsum().fillna(0.0)
    yaxis = "Physics time"
    xaxis = df.filter(regex="^(?!.*time).*$").drop("Timestep", axis=1).columns

    titles = [f"{yaxis} VS {x}" for x in xaxis]

    marker = dict(color=color) if color is not None else None
    return {
        title: go.Figure(
            data=[
                go.Scatter(
                    x=df[x], y=df[yaxis], mode="markers", marker=marker, name=name
                )
            ],
            layout_title=title,
        )
        for title, x in zip(titles, xaxis)
    }


def generate_report(data: tuple[Path, Any], csvs: dict[Path, pd.DataFrame]) -> None:
    output_folder, summary = data
    print(f"Generating report for '{output_folder}'")

    summary = {output_folder.name: summary}
    content = write_markdown_recursive(summary)
    if output_folder in csvs:
        time_unit = summary[output_folder.name]["Performance summary"]["Unit"]

        content += f"\n## Benchmark plots ({time_unit})\n"
        for filename, fig in create_plot_from_df(csvs[output_folder]).items():
            print(f"    Writing '{filename}' plot")
            fig.write_html(output_folder / f"{filename}.html")
            fig.write_image(output_folder / f"{filename}.png")
            content += f"![{filename}](./{filename}.png)\n"

    md = markdown.markdown(content)
    with open(output_folder / "index.md", "w") as ffile:
        ffile.write(content)

    with open(output_folder / "index.html", "w") as ffile:
        ffile.write(md)


def generate_combined_report(
    summaries: dict[Path, Any], csvs: dict[Path, pd.DataFrame], args: Namespace
) -> None:
    print("Generating combine report")
    big_summary: dict[str, Any] = {}
    plots: dict[str, go.Figure] = {}

    plotly_colors = ["#1f77b4", "#ff7f0e", "#2ca02c", "#d62728", "#9467bd", "#8c564b"]
    cindex = 0
    for output_folder, summary in summaries.items():
        big_summary[output_folder.name] = summary
        if output_folder in csvs:
            for title, fig in create_plot_from_df(
                csvs[output_folder],
                color=plotly_colors[cindex],
                name=output_folder.name,
            ).items():
                if title not in plots:
                    plots[title] = fig
                else:
                    for trace in fig.data:
                        plots[title].add_trace(trace)
            cindex = (cindex + 1) % len(plotly_colors)

    content = write_markdown_recursive(big_summary)

    def write_figures(data: tuple[str, go.Figure]) -> None:
        filename, fig = data
        print(f"    Writing '{filename}' plot")
        fig.write_html(args.output / f"{filename}.html")
        fig.write_image(args.output / f"{filename}.png")

    if args.jobs is not None:
        with mp.Pool(args.jobs) as pool:
            pool.map(write_figures, plots.items())
        for filename, fig in plots.items():
            content += f"![{filename}](./{filename}.png)\n"
    else:
        for filename, fig in plots.items():
            write_figures((filename, fig), content)
            content += f"![{filename}](./{filename}.png)\n"

    md = markdown.markdown(content)
    with open(args.output / "index.md", "w") as ffile:
        ffile.write(content)

    with open(args.output / "index.html", "w") as ffile:
        ffile.write(md)


def main() -> None:
    t1 = perf_counter()
    args = create_and_parse_args()

    output_path: Path = args.output.resolve()
    if args.combine:
        if output_path.exists() and not args.regenerate:
            print(f"Done in {perf_counter() - t1:.2f} seconds")
            return
        output_path.mkdir(parents=True, exist_ok=True)

    if not output_path.is_dir():
        raise ValueError(f"Output path '{output_path}' is not a directory")

    csvs: dict[Path, pd.DataFrame] = {}
    summaries: dict[Path, Any] = {}
    for input_path in args.input:
        input_path: Path = input_path.resolve()
        if not input_path.is_dir():
            raise ValueError(f"Input path '{input_path}' is not a directory")

        for f in input_path.rglob("*"):
            if f.is_dir() or f.name.startswith("."):
                continue

            key = output_path / f.parent.name
            if not args.combine:
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
            pool.map(fn, summaries.items())
    else:
        for output_folder, summary in summaries.items():
            generate_report((output_folder, summary), csvs)

    print(f"Done in {perf_counter() - t1:.2f} seconds")


if __name__ == "__main__":
    main()
