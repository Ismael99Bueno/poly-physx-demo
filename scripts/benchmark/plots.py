from collections.abc import Sequence

import plotly.graph_objects as go
import pandas as pd


def create_plot_from_df(
    df: pd.DataFrame,
    xreg_labels: Sequence[str],
    yreg_labels: Sequence[str],
    log_seq: str | None,
    color=None,
    name: str = None,
) -> dict[str, go.Figure]:
    log = (
        [c == "1" for c in log_seq]
        if log_seq is not None
        else [False] * len(xreg_labels)
    )
    if len(log) != len(xreg_labels):
        raise ValueError(
            "The number of log values must be the same as the number of x labels"
        )

    df["app_time"] = df["app_timestep"].cumsum().fillna(0.0)
    df["physics_time"] = df["physics_timestep"].cumsum().fillna(0.0)
    plots: dict[str, go.Figure] = {}
    marker = {"size": 3}
    if color is not None:
        marker["color"] = color

    for xreg, yreg, is_log in zip(xreg_labels, yreg_labels, log):
        for col1 in df.filter(regex=xreg).columns:
            for col2 in df.filter(regex=yreg).columns:
                if col1 != col2:
                    title = f"{col1} VS {col2}"
                    plots[title] = go.Figure(
                        data=[
                            go.Scatter(
                                x=df[col1],
                                y=df[col2],
                                mode="markers",
                                marker=marker,
                                name=name,
                            )
                        ],
                        layout_title=title,
                    )
                    if is_log:
                        plots[title].update_layout(yaxis={"type": "log"})
    return plots
