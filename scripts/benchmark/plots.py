import plotly.graph_objects as go
import pandas as pd


def create_plot_from_df(
    df: pd.DataFrame, color=None, name: str = None, log: bool = False
) -> dict[str, go.Figure]:
    df["Time"] = df["APP timestep"].cumsum().fillna(0.0)
    yaxis = "Physics time"
    xaxis = (
        df.filter(regex="^(?!.*time).*$")
        .drop(["APP timestep", "Physics timestep"], axis=1, errors="ignore")
        .columns
    )

    titles = [f"{yaxis} VS {x}" for x in xaxis]

    marker = {"color": color, "size": 3} if color is not None else None
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
    plots["total-collisions-vs-positive-collisions"] = go.Figure(
        data=[
            go.Scatter(
                x=df["Positive collision checks"],
                y=df["Total collision checks"],
                mode="markers",
                marker=marker,
                name=name,
            )
        ],
        layout_title="Total collisions VS Positive collisions",
    )
    if log:
        for k in plots:
            plots[k].update_layout(yaxis={"type": "log"})
    plots["total-contacts-vs-bodies"] = go.Figure(
        data=[
            go.Scatter(
                x=df["Bodies"],
                y=df["Total contacts"],
                mode="markers",
                marker=marker,
                name=name,
            )
        ],
        layout_title="Total contacts VS Bodies",
    )
    return plots
