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

datait = pd.read_csv(sys.argv[1], sep=" ")
datait.columns = [
    "mon",
    "traces_num",
    "traces_len",
    "max_wbg_size",
    "cputime",
    "walltime",
    "mem",
    "returncode",
]

f, axs = plt.subplots(1, 3, figsize=(10, 3))

ycol = "cputime"
xcol = "traces_num"
plot = sns.lineplot(
    data=datait,
    x=xcol,
    y=ycol,
    hue="traces_len",
    ax=axs[0],
    palette="deep",
    markers=True,
    dashes=False,
)
plot.legend(fontsize=9, title="Length of traces")
axs[0].set(xlabel="Number of traces", ylabel="CPU time [s]")

ax = axs[1]
ycol = "mem"
xcol = "traces_num"
plot = sns.lineplot(
    data=datait,
    x=xcol,
    y=ycol,
    hue="traces_len",
    ax=ax,
    palette="deep",
    markers=True,
    dashes=False,
)
plot.legend(fontsize=9, title="Length of traces")
ax.legend().set_visible(False)
ax.set(xlabel="Number of traces", ylabel="Memory [MB]")

ax = axs[2]
ycol = "max_wbg_size"
xcol = "traces_num"
plot = sns.lineplot(
    data=datait,
    x=xcol,
    y=ycol,
    hue="traces_len",
    ax=ax,
    palette="deep",
    markers=True,
    dashes=False,
)
plot.legend(fontsize=9, title="Length of traces")
ax.legend().set_visible(False)
ax.set(xlabel="Number of traces", ylabel="Maximal size of workbag")
#

fig = plot.get_figure()
fig.tight_layout()
fig.savefig(join(topdir, "plot-rand.pdf"), bbox_inches="tight", dpi=600)
