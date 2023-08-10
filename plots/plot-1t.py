#!/usr/bin/python3

import sys
import csv
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from statistics import mean
import pandas as pd
import seaborn as sns
from os.path import basename, dirname, join

topdir = join(dirname(__file__), "..")


def get_param(mon):
    # monitor_1t_40-400-100
    name = mon.split("-", 2)[0]
    return 0, int(name.split("_")[2])
    # return int(parts[2]), int(parts[3][:-4])


def snd_p(mon):
    return get_param(mon)[1]


frames = []

data1 = pd.read_csv(sys.argv[1], sep=" ")
data1.columns = [
    "mon",
    "traces_num",
    "Length of traces",
    "max_wbg_size",
    "cputime",
    "walltime",
    "mem",
    "returncode",
]
data1["type"] = data1["mon"].map(get_param)
data1["shift"] = 0
data1["Distance"] = data1["mon"].map(snd_p)

data = data1

#####################################################################
f, axs = plt.subplots(1, 2, figsize=(10, 3), width_ratios=[2, 1])

ax = axs[0]
ycol = "cputime"
xcol = "traces_num"
D = data1[data1["Distance"].isin(((10, 100, 190)))]
plot = sns.lineplot(
    data=D,
    x=xcol,
    y=ycol,
    hue="Length of traces",
    style="Distance",
    ax=ax,
    palette="deep",
    markers=True,
    dashes=True,
    errorbar=None,
)

plot.legend(fontsize=9, ncol=2)

ax.set(
    xlabel="Number of traces",
    ylabel="CPU time [s]",
)

#####################################################################
ax = axs[1]
ycol = "mem"
xcol = "traces_num"
plot = sns.lineplot(
    data=D,
    x=xcol,
    y=ycol,
    hue="Length of traces",
    style="Distance",
    ax=ax,
    palette="deep",
    markers=True,
    dashes=False,
    errorbar=None,
)  # , ci="sd")
plot.legend(fontsize=9, title="Length of traces")
ax.set(
    xlabel="Number of traces",
    ylabel="Memory [MB]",
)

fig = plot.get_figure()
fig.tight_layout()
fig.savefig(join(topdir, "plot-1t.pdf"), bbox_inches="tight", dpi=300)
