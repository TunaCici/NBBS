import argparse
from random import randint

from numpy import arange, linspace, pi, sin

from bokeh.layouts import column
from bokeh.models import (CustomJS, LinearAxis, Range1d, Select,
                          WheelZoomTool, ZoomInTool, ZoomOutTool)
from bokeh.palettes import Sunset6
from bokeh.plotting import figure, show

def random_color():
    return "#%06x" % randint(0, 0xFFFFFF)

def stress_multi_graph(threads : list):
    time_points = [] # x-axis
    latencies = [] # left y-axis
    memory_usages = [] # right y-axis

    x_range = ()
    y_left_range = ()
    y_right_range = ()

    background_fill_color = "#FAFAFA"

    # Use the first thread as a reference
    for idx, operation in enumerate(threads[0]):
        time_points.append(idx)
        memory_usages.append(operation[1] * 100)
        
    x_range = (min(time_points), max(time_points))
    y_left_range = (0, 2000) # 0 - 1000 us
    y_right_range = (0, 100) # 0 - 100 %

    p = figure(x_range=x_range, y_range=y_left_range, tools="pan,box_zoom,save,reset", width=1200, height=600)

    # Draw time points & memory usages
    p.xaxis.axis_label = "Time Points"
    p.yaxis.axis_label = "Memory Usage (%)"

    p.extra_y_ranges["memory_usages"] = Range1d(y_right_range[0], y_right_range[1])
    p.add_layout(LinearAxis(y_range_name="memory_usages", axis_label="Memory Usages (%)"), "right")
    p.line(time_points, memory_usages, legend_label="Memory Usage (%)", line_width=2, color="green", y_range_name="memory_usages")

    # Draw latencies for each thread
    for idx, thread in enumerate(threads):
        for operation in thread:
            latencies.append(operation[0])
        p.line(time_points, latencies[:len(time_points)], legend_label=f"Latencies (us) - Thread {idx}", line_width=1, color=random_color())
        
        latencies.clear()

    # Plot configuration
    wheel_zoom = WheelZoomTool()
    p.add_tools(wheel_zoom)
    p.toolbar.active_scroll = wheel_zoom

    show(p)

if __name__ == "__main__":
    # 0. Parse arguments
    parser = argparse.ArgumentParser(description='Draw graphs from ./bench results using Bokeh')
    parser.add_argument('--input', type=str, default="results.txt", help='Path to benchmark results (default: results.txt)')
    args = parser.parse_args()

    # 1. Read into memory
    lines = []
    with open(args.input, "r", encoding="utf-8") as results:
        lines = [line.rstrip() for line in results]

    # 2. Parse
    benchmark = lines[0]
    threads = []

    for line in lines:
        if line.startswith("thread"):
            threads.append([])

            operations = line[(line.find(":")) + 1: ].split("), ")

            for operation in operations:
                latency = int(operation[operation.find("(") +1: operation.find("us")])
                mem_usage = float(operation[operation.find(",") +2: operation.find("%")])

                threads[-1].append([latency, mem_usage])

    # 3. Call the corresponding function
    stress_multi_graph(threads)
