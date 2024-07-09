import plotly.graph_objects as go
import pandas as pd


def create_plot_from_df(
    df: pd.DataFrame, color=None, name: str = None
) -> dict[str, go.Figure]:
    df["Time"] = df["Timestep"].cumsum().fillna(0.0)
    yaxis = "Physics time"
    xaxis = df.filter(regex="^(?!.*time).*$").drop("Timestep", axis=1).columns

    titles = [f"{yaxis} VS {x}" for x in xaxis]

    marker = dict(color=color) if color is not None else None
    return {
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
