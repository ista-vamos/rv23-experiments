#!/usr/bin/env python3

from subprocess import Popen, PIPE
from os.path import dirname, realpath, basename
from os import listdir, access, X_OK
from sys import argv
from multiprocessing import Pool, Lock

bindir = f"{dirname(realpath(__file__))}/bin"
TIMEOUT = 120

lock = Lock()

def run_one(arg):
    binary, n, l = arg
    cmd = ["/bin/time", "-f", '%Uuser %Ssystem %eelapsed %PCPU (%Xavgtext+%Davgdata %Mmaxresident)k',
           f"{bindir}/{binary}"]
    #print(cmd)

    p = Popen(cmd, stderr=PIPE, stdout=PIPE)
    try:
        out, err = p.communicate(timeout=TIMEOUT)
    except TimeoutExpired:
        p.kill()
        out, err = p.communicate()
    #assert p.returncode in (0, 1), p
    #assert out is not None, cmd
    assert err is not None, cmd

    # Max workbag size: 7391
    #Traces #: 500
    #1.73user 0.03system 01.76elapsed 99%CPU (0avgtext+0avgdata 137604maxresident)k
    wbg_size=None
    cpu_time=None
    wall_time=None
    mem=None
    for line in out.splitlines():
        line = line.strip()
        if line.startswith(b"Max workbag"):
            wbg_size = int(line.split()[3])

    for line in err.splitlines():
        if b"elapsed" in line:
            parts = line.split()
            assert b"user" in parts[0]
            assert b"elapsed" in parts[2]
            assert b"maxresident" in parts[5]
            cpu_time = float(parts[0][:-4])
            wall_time = float(parts[2][:-7])
            mem = int(parts[5][:-13])/1024.0

    with lock:
        print(binary, n, l, wbg_size, cpu_time, wall_time, mem, p.returncode)
    #return (n, l, wbg_size, cpu_time, wall_time, mem)

def get_params(binary):
    for fl in listdir(bindir):
        if fl.startswith(binary) and access(f"{bindir}/{fl}", X_OK):
                # monitor_1t_150-500-100
                parts = fl.split("-")
                assert len(parts) == 3, parts
                #print(basename(fl), int(parts[1]), int(parts[2]))
                yield (basename(fl), int(parts[1]), int(parts[2]))

binary = argv[1]
proc_num=None
if len(argv) == 3:
    proc_num=int(argv[2])

with Pool(processes=proc_num) as pool:
    result = pool.map(run_one, get_params(binary))

