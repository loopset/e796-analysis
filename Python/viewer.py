import psutil, datetime
import pandas as pd
import dash
from dash import callback, dcc, html
from dash.dependencies import Input, Output
from plotly.subplots import make_subplots
import plotly.graph_objs as go

# Viewer settings
interval = 1000  # ms
max_range = 20 * 60  # seconds

# Data buffer
df = pd.DataFrame(
    {
        "time": pd.Series(dtype="datetime64[ns]"),
        "cpu": pd.Series(dtype="float"),
        "ram": pd.Series(dtype="float"),
    }
)

# Build figure
fig = make_subplots(
    rows=2, cols=1, shared_xaxes=True, subplot_titles=("CPU (%)", "RAM (%)")
)
# CPU
fig.add_trace(
    go.Scatter(x=df["time"], y=df["cpu"], mode="lines", name="CPU"), row=1, col=1
)
# RAM
fig.add_trace(
    go.Scatter(x=df["time"], y=df["ram"], mode="lines", name="RAM"), row=2, col=1
)

fig.update_layout(
    template="gridon", showlegend=False, uirevision="constant"
)

# Main Dash app
app = dash.Dash(__name__)
app.layout = html.Div(
    [
        html.H2("Simple viewer"),
        dcc.Graph(id="usage_graph", figure=fig),
        dcc.Interval(id="interval", interval=interval, n_intervals=0),
    ]
)


@app.callback(Output("usage_graph", "figure"), Input("interval", "n_intervals"))
def update(_):
    global df, fig
    now = datetime.datetime.now()
    new_row = {
        "time": now,
        "cpu": psutil.cpu_percent(),
        "ram": psutil.virtual_memory().percent,
    }
    df = pd.concat([df, pd.DataFrame([new_row])], ignore_index=True)

    # Keep only recend data
    mask = now - datetime.timedelta(seconds=max_range)
    df = df[df["time"] >= mask]

    # And update them
    fig.data[0].x = df["time"]
    fig.data[0].y = df["cpu"]
    fig.data[1].x = df["time"]
    fig.data[1].y = df["ram"]

    if fig.layout.xaxis.range is None or fig.layout.xaxis.range[1] < mask:
        fig.update_xaxes(range=[mask, now])

    return fig


if __name__ == "__main__":
    app.run(debug=True)
