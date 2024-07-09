from pathlib import Path
from argparse import Namespace
from typing import Any

from md import write_markdown_recursive
from plots import create_plot_from_df

import pandas as pd
import plotly.graph_objects as go
import multiprocess as mp
import markdown


def generate_report(
    output_folder: Path, summary, csvs: dict[Path, pd.DataFrame]
) -> None:
    print(f"Generating report at '{output_folder}'")

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

    def write_figures(filename: str, fig: go.Figure) -> None:
        print(f"    Writing '{filename}' plot")
        fig.write_html(args.output / f"{filename}.html")
        fig.write_image(args.output / f"{filename}.png")

    if args.jobs is not None:
        with mp.Pool(args.jobs) as pool:
            pool.starmap(write_figures, plots.items())
        for filename, fig in plots.items():
            content += f"![{filename}](./{filename}.png)\n"
    else:
        for filename, fig in plots.items():
            write_figures(filename, fig)
            content += f"![{filename}](./{filename}.png)\n"

    md = markdown.markdown(content)
    with open(args.output / "index.md", "w") as ffile:
        ffile.write(content)

    with open(args.output / "index.html", "w") as ffile:
        ffile.write(md)
