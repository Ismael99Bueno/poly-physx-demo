import plotly.graph_objects as go
import pandas as pd


def create_plot_from_df(
    df: pd.DataFrame, color=None, name: str = None
) -> dict[str, go.Figure]:
    df["Time"] = df["APP timestep"].cumsum().fillna(0.0)
    yaxis = "Physics time"
    xaxis = (
        df.filter(regex="^(?!.*time).*$")
        .drop(["APP timestep", "Physics timestep"], axis=1, errors="ignore")
        .columns
    )

    titles = [f"{yaxis} VS {x}" for x in xaxis]

    marker = dict(color=color) if color is not None else None
    plots = {
        title.lower().replace(" ", "-"): go.Figure(
            data=[
                go.Scatter(
                    x=df[x], y=df[yaxis], mode="markers", marker=marker, name=name
                )
            ],
            layout_title=title,
        )
        for title, x in zip(titles, xaxis)
    }
    plots["bodies-vs-total-contacts"] = go.Figure(
        data=[
            go.Scatter(
                x=df["Total contacts"],
                y=df["Bodies"],
                mode="markers",
                marker=marker,
                name=name,
            )
        ],
        layout_title="Bodies VS Total contacts",
    )
    return plots
