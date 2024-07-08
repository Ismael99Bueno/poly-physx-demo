from pathlib import Path
from typing import Any
from plotly.graph_objs import Figure
from time import perf_counter

import plotly.express as px
import pandas as pd
import multiprocess as mp
import yaml
import markdown
import shutil
import functools


def write_markdown_recursive(summary: dict, content: str = "", depth: int = 1) -> str:
    for key, value in summary.items():
        if isinstance(value, dict):
            content += f"{'#' * depth} {key}\n"
            content = write_markdown_recursive(value, content, depth + 1)
        else:
            content += f"- {key}: {value}\n"
    return content


def create_plot_from_csv(df: pd.DataFrame) -> dict[str, Figure]:
    df["Time"] = df["Timestep"].cumsum().fillna(0.0)
    yaxis = "Physics time"
    xaxis = df.filter(regex="^(?!.*time).*$").drop("Timestep", axis=1).columns

    titles = [f"{yaxis} VS {x}" for x in xaxis]
    return {
        title: px.scatter(df, x=x, y=yaxis, title=title)
        for title, x in zip(titles, xaxis)
    }


def generate_report(data: tuple[Path, Any], csvs: dict[Path, pd.DataFrame]) -> None:
    output_folder, summary = data
    print(f"Generating report for '{output_folder}'")

    content = write_markdown_recursive(summary)
    if output_folder in csvs:
        time_unit = summary["Performance summary"]["Unit"]

        content += f"\n## Benchmark plots ({time_unit})\n"
        for filename, fig in create_plot_from_csv(csvs[output_folder]).items():
            print(f"    Writing '{filename}' plots")
            fig.write_html(output_folder / f"{filename}.html")
            fig.write_image(output_folder / f"{filename}.png")
            content += f"![{filename}](./{filename}.png)\n"

    md = markdown.markdown(content)
    with open(output_folder / "index.md", "w") as file:
        file.write(content)

    with open(output_folder / "index.html", "w") as file:
        file.write(md)


def main() -> None:
    t1 = perf_counter()

    benchmark_data = Path("output") / "benchmark-data"
    reports_folder = Path("output") / "benchmark-reports"
    reports_folder.mkdir(parents=True, exist_ok=True)

    csvs: dict[Path, pd.DataFrame] = {}
    summaries: dict[Path, Any] = {}
    for f in benchmark_data.rglob("*"):
        if f.is_dir() or f.name.startswith("."):
            continue

        output_folder = reports_folder / f.relative_to(benchmark_data).parent
        shutil.rmtree(output_folder)

        output_folder.mkdir(parents=True, exist_ok=True)

        if f.suffix == ".csv":
            data = pd.read_csv(f.absolute())
            csvs[output_folder] = data
        else:
            with open(f, "r") as file:
                data = yaml.safe_load(file)
            summaries[output_folder] = data
    multithreading = True

    if multithreading:
        fn = functools.partial(generate_report, csvs=csvs)
        with mp.Pool(mp.cpu_count()) as pool:
            pool.map(fn, summaries.items())
    else:
        for output_folder, summary in summaries.items():
            generate_report((output_folder, summary), csvs)

    print(f"Done in {perf_counter() - t1:.2f} seconds")


if __name__ == "__main__":
    main()
