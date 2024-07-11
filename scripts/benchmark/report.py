from pathlib import Path
from argparse import Namespace
from typing import Any
from collections.abc import Mapping

from md import write_markdown_recursive
from plots import create_plot_from_df

import pandas as pd
import plotly.graph_objects as go
import multiprocess as mp
import markdown
import os


def generate_report(
    output_folder: Path, summary, csvs: Mapping[Path, pd.DataFrame], args: Namespace
) -> None:
    print(f"Generating report at '{output_folder}'")
    if args.regenerate and output_folder.exists():
        for f in output_folder.glob("*"):
            if f.is_file():
                f.unlink()

    summary = {output_folder.name: summary}
    content = write_markdown_recursive(summary)
    if output_folder in csvs:
        output_folder.mkdir(parents=True, exist_ok=True)
        time_unit = __short_unit(
            summary[output_folder.name]["Performance summary"]["Unit"]
        )

        content += f"\n## Benchmark plots ({time_unit})\n"
        for filename, fig in create_plot_from_df(csvs[output_folder]).items():
            html_path = output_folder / f"{filename}.html"
            png_path = output_folder / f"{filename}.png"

            if not html_path.exists() or args.overwrite:
                print(f"    Writing '{html_path}'")
                fig.write_html(output_folder / f"{filename}.html")

            if not png_path.exists() or args.overwrite:
                print(f"    Writing '{png_path}'")
                fig.write_image(output_folder / f"{filename}.png")
            content += f"![{filename}](./{filename}.png)\n"

    md_path = output_folder / "index.md"
    html_path = output_folder / "index.html"

    if not md_path.exists() or args.overwrite:
        md = markdown.markdown(content)
        with open(output_folder / "index.md", "w") as ffile:
            ffile.write(content)

    if not html_path.exists() or args.overwrite:
        with open(output_folder / "index.html", "w") as ffile:
            ffile.write(md)


def generate_combined_report(
    summaries: Mapping[Path, Any], csvs: Mapping[Path, pd.DataFrame], args: Namespace
) -> None:
    print("Generating combined report")
    if args.regenerate and args.output.exists():
        for f in args.output.glob("*"):
            if f.is_file():
                f.unlink()

    big_summary: dict[Path, Any] = {}
    plots: dict[str, go.Figure] = {}

    plotly_colors = [
        "#1f77b4",
        "#ff7f0e",
        "#2ca02c",
        "#d62728",
        "#9467bd",
        "#8c564b",
        "#e377c2",
        "#7f7f7f",
        "#bcbd22",
        "#17becf",
    ]
    cindex = 0
    common_path = os.path.commonpath(csvs.keys())

    for parent, summary in summaries.items():
        big_summary[parent.relative_to(common_path)] = summary

        if parent in csvs:
            time_unit = __short_unit(summary["Performance summary"]["Unit"])
            shortened_name = str(parent.relative_to(common_path))
            if len(shortened_name) > 12:
                shortened_name = f"...{shortened_name[-9:]}"

            for title, fig in create_plot_from_df(
                csvs[parent],
                color=plotly_colors[cindex],
                name=f"{shortened_name} ({time_unit})",
            ).items():
                if title not in plots:
                    plots[title] = fig
                else:
                    for trace in fig.data:
                        plots[title].add_trace(trace)
            cindex = (cindex + 1) % len(plotly_colors)

    content = write_markdown_recursive(big_summary)

    def write_figures(filename: str, fig: go.Figure) -> None:
        html_path = args.output / f"{filename}.html"
        png_path = args.output / f"{filename}.png"

        if not html_path.exists() or args.overwrite:
            print(f"    Writing '{html_path}'")
            fig.write_html(args.output / f"{filename}.html")

        if not png_path.exists() or args.overwrite:
            print(f"    Writing '{png_path}'")
            fig.write_image(args.output / f"{filename}.png")

    content += "\n## Benchmark plots\n"
    if args.jobs is not None:
        with mp.Pool(args.jobs) as pool:
            pool.starmap(write_figures, plots.items())
        for filename, fig in plots.items():
            content += f"![{filename}](./{filename}.png)\n"
    else:
        for filename, fig in plots.items():
            write_figures(filename, fig)
            content += f"![{filename}](./{filename}.png)\n"

    md_path = args.output / "index.md"
    html_path = args.output / "index.html"

    if not md_path.exists() or args.overwrite:
        md = markdown.markdown(content)
        with open(args.output / "index.md", "w") as ffile:
            ffile.write(content)

    if not html_path.exists() or args.overwrite:
        with open(args.output / "index.html", "w") as ffile:
            ffile.write(md)


def __short_unit(unit: str) -> str:
    return {
        "Nanoseconds": "ns",
        "Microseconds": "µs",
        "Milliseconds": "ms",
        "Seconds": "s",
    }[unit]
