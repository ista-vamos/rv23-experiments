#!/usr/bin/env python3

import glob
from subprocess import Popen, PIPE
from os.path import dirname, realpath, basename
from os import listdir
from sys import argv
from multiprocessing import Pool

def run_one(arg):
    binary, args = arg
    cmd = ["/bin/time", f"{binary}"] + args
    #print(cmd)

    p = Popen(cmd, stderr=PIPE, stdout=PIPE)
    out, err = p.communicate()
    assert p.returncode == 0, p
    assert err is not None, cmd
    assert out is not None, cmd

    # Max workbag size: 7391
    #Traces #: 500
    #1.73user 0.03system 0:01.76elapsed 99%CPU (0avgtext+0avgdata 137604maxresident)k
    #0inputs+0outputs (0major+34642minor)pagefaults 0swaps
    traces=None
    wbg_size=None
    cpu_time=None
    wall_time=None
    mem=None
    for line in out.splitlines():
        line = line.strip()
        if line.startswith(b"Max workbag"):
            wbg_size = int(line.split()[3])
        if line.startswith(b"Traces"):
            traces = int(line.split()[2])

    for line in err.splitlines():
        if b"elapsed" in line:
            parts = line.split()
            assert b"user" in parts[0]
            assert b"elapsed" in parts[2]
            assert b"maxresident" in parts[5]
            cpu_time = float(parts[0][:-4])
            wall_time = float(parts[2][2:-7])
            mem = int(parts[5][:-13])/1024

    print(traces, wbg_size, cpu_time, wall_time, mem)

def get_params(datadir):
    for i in range(1, 22):
        yield [fl for pattern in (f"{datadir}/{j:0>2}-*.txt" for j in range(1, i+1)) for fl in glob.glob(pattern) ]

binary = argv[1]
datadir = argv[2]

proc_num=None
if len(argv) == 4:
    proc_num=int(argv[3])

with Pool(processes=proc_num) as pool:
    result = pool.map(run_one, ((binary, args) for args in get_params(datadir)))

