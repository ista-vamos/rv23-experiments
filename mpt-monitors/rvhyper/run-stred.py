#!/usr/bin/env python3

import datetime
from tempfile import mkdtemp
from subprocess import Popen, PIPE, DEVNULL, run as runcmd, TimeoutExpired
from os.path import dirname, realpath, basename, abspath, join, isfile, isdir
import os
from os import listdir, access, X_OK, environ as ENV, symlink, makedirs
from sys import argv, stderr, stdout
from multiprocessing import Pool, Lock
from shutil import rmtree
import signal

import argparse

lock = Lock()

bindir = f"{dirname(realpath(__file__))}/"
mpt_binary = join(bindir, "monitor-rvhyper")
rvhyper_dir = join(bindir, "rvhyper")
hnl_dir = join(bindir, "hnl")
SPOT_LIBDIR="/home/xchalup4/ifm24/rv23-experiments/mpt-monitors/rvhyper/rvhyper/spot-install/lib/"

TIMEOUT = 120
# Do not generate `traces_num` random traces, but generate just one
# and use it `traces_num` times (this will force the monitors to
# read entire traces to the end)
REPEAT_ONE_TRACE = False
TRIALS = 5

TRACES_LEN  = [1000]
TRACES_NUMS = [200, 400, 600, 800]
# HNL is up to 10 bits
BITS = [8]

def errlog(*args):
    with open(join(dirname(__file__), "log.txt"), "a") as logf:
        for a in args:
            print(a, file=logf)

def run_one(arg):
    traces_num, trace_len, bits = arg
    traces_dir = mkdtemp(prefix="/tmp/")

    # -- GENERATE TRACES
    with lock:
        print(f".. [{datetime.datetime.now().time()}] running # traces = {traces_num}, len = {trace_len}, bits = {bits}", file=stderr)
        stdout.flush()

    runcmd(["python", f"{bindir}/gen-traces.py",
            str(1 if REPEAT_ONE_TRACE else traces_num), str(trace_len), str(bits), f"force-od,no-stuttering,outdir={traces_dir}"],
            stderr=DEVNULL,
            stdout=DEVNULL, check=True)

    # get the list of files
    if REPEAT_ONE_TRACE:
        files = ["1.tr"] * traces_num
    else:
        files = []
        for fl in listdir(traces_dir):
            if fl.endswith(".tr"):
                files.append(fl)

    # -- RUN MONITORS
    for n in range(TRIALS):
        # run eHL monitor
        run_hnl(arg, traces_dir, files)

        # run eHL monitor
        run_hnl(arg, traces_dir, files, stred=True)


    try:
        rmtree(traces_dir)
    except Exception as e:
        print("Failed removing traces: ", e, file=stderr)
        rmtree(traces_dir, ignore_errors=True)

def run_hnl(arg, traces_dir, files, stred=False):
    traces_num, trace_len, bits = arg
    stred = '-stred' if stred else ''
    cmd = ["/bin/time", "-f", '%Uuser %Ssystem %eelapsed %PCPU (%Xavgtext+%Davgdata %Mmaxresident)k',
           join(f"{hnl_dir}-{bits}b{stred}", "monitor")]
    cmd += files
    p = Popen(cmd, stderr=PIPE, stdout=PIPE, cwd=traces_dir, preexec_fn=os.setsid)
    try:
        out, err = p.communicate(timeout=TIMEOUT)
    except TimeoutExpired:
        os.killpg(os.getpgid(p.pid), signal.SIGTERM) 
        os.killpg(os.getpgid(p.pid), signal.SIGKILL) 
       #p.terminate()
       #p.kill()
        out, err = p.communicate(timeout=10)
    #assert p.returncode == 0, p
    # assert out is not None, cmd
    assert err is not None, cmd

    cpu_time=None
    wall_time=None
    mem=None
    instances, atoms, reused_mons, reused_verdicts = None, None, None, None
    verdict = None
    if p.returncode in (0, 1):
        for line in out.splitlines():
            line = line.strip()
            if line.startswith(b"Total formula"):
                instances = int(line.split()[3])
            elif line.startswith(b"Total atom"):
                atoms = int(line.split()[3])
            elif line.startswith(b"Reused monitors"):
                reused_mons = int(line.split()[2])
            elif line.startswith(b"Reused verdicts"):
                reused_verdicts = int(line.split()[2])
            elif b'TRUE' in line:
                verdict = 'TRUE'
            elif b'FALSE' in line:
                verdict = 'FALSE'

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
        print(f"hnl{stred}", traces_dir, traces_num, trace_len, bits, verdict, instances, atoms, reused_mons, reused_verdicts, cpu_time, wall_time, mem, p.returncode)
        stdout.flush()
    #return (n, l, wbg_size, cpu_time, wall_time, mem)




def get_params():
    for N in TRACES_NUMS:
    	for L in TRACES_LEN:
    	    for B in BITS:
                yield N, L, B

def run(args):
    print(f"\033[1;34mRunning using {args.j} workers\033[0m", file=stderr)
    with Pool(processes=args.j) as pool:
        result = pool.map(run_one, get_params())

parser = argparse.ArgumentParser()
parser.add_argument("--one-trace", action='store_true')
parser.add_argument("-j", metavar="PROC_NUM", action='store', type=int)
#parser.add_argument("--traces-dir", help="Take traces from this dir. If the dir does not exists, generate traces to this dir", action='store')
args = parser.parse_args()
#if args.traces_dir:
#    args.traces_dir = abspath(args.traces_dir)

if args.one_trace:
    REPEAT_ONE_TRACE = True

print("Repeating the same trace", file=stderr)
run(args)
