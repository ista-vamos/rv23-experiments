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
MEMLIMIT = 8589934592  # bytes, which is 8GB
# Do not generate `traces_num` random traces, but generate just one
# and use it `traces_num` times (this will force the monitors to
# read entire traces to the end)
REPEAT_ONE_TRACE = False
TRIALS = 5

TRACES_LEN  = [500, 1000, 2000, 3000, 4000, 5000]
TRACES_NUMS = [500, 1000, 2000, 3000, 4000, 5000]
# HNL is up to 10 bits
#BITS = (1, 2, 4, 8, 10)
BITS = (2, 8, 10)

RUNEXEC = ["runexec", "--no-container", "--read-only-dir", "/", "--timelimit",  str(TIMEOUT), "--memlimit", str(MEMLIMIT), "--output", "/dev/stdout", "--"]

def errlog(*args):
    with open(join(dirname(__file__), "log.txt"), "a") as logf:
        for a in args:
            print(a, file=logf)

def run_one(arg):
    traces_num, trace_len, bits = arg
    traces_dir = mkdtemp(prefix="/tmp/")

    # -- GENERATE TRACES
    if REPEAT_ONE_TRACE:
        traces_num = 1

    with lock:
        print(f".. [{datetime.datetime.now().time()}] running # traces = {traces_num}, len = {trace_len}, bits = {bits}", file=stderr)
        stdout.flush()

    runcmd(["python", f"{bindir}/gen-traces.py",
            str(traces_num), str(trace_len), str(bits), f"force-od,outdir={traces_dir}"],
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
        # run MPT monitor
        run_mpt(arg, traces_dir, files)

        # run rvhyper
        run_rvhyper(arg, traces_dir, files)

        # run eHL monitor
        run_hnl(arg, traces_dir, files)

        # run rvhyper with --sequential
        #run_rvhyper(arg, files, ["--sequential"], "-seq")

    try:
        rmtree(traces_dir)
    except Exception as e:
        print("Failed removing traces: ", e, file=stderr)
        rmtree(traces_dir, ignore_errors=True)


def run_rvhyper(arg, traces_dir, files, rvh_args=None, name_suffix=""):
    traces_num, trace_len, bits = arg
    rvh = join(rvhyper_dir, "build/release/rvhyper")
    assert access(rvh, X_OK), f"Cannon find rvhyper binary, assumed is {rvh}"
    cmd = [rvh]
    if rvh_args:
        cmd += rvh_args
    cmd += ["-S", f"{traces_dir}/od-{bits}b.hltl"] + files
    # print("> ", " ".join(cmd))

    # symlink `eahyper` to the working directory, rvhyper assumes it there
    eahyper_link = join(traces_dir, "eahyper")
    try:
        symlink(join(rvhyper_dir, "eahyper"), eahyper_link)
    except FileExistsError:
        pass

    env = ENV.copy()
    env["EAHYPER_SOLVER_DIR"] = join(rvhyper_dir, "LTL_SAT_solver")
    env["LD_LIBRARY_PATH"] = ":".join([join(rvhyper_dir, "lib"),
                                       SPOT_LIBDIR])

    p = Popen(RUNEXEC + cmd, stderr=PIPE, stdout=PIPE, cwd=traces_dir, env=env, preexec_fn=os.setsid)
    try:
        out, err = p.communicate(timeout=TIMEOUT)
        if p.returncode != 0:
            errlog(env, p, out, err)
    except TimeoutExpired:
        os.killpg(os.getpgid(p.pid), signal.SIGTERM) 
        out, err = p.communicate(timeout=10)


    #print(p, p.returncode, out, err)
    #assert p.returncode == 0, p
    assert err is not None, cmd
    #assert out is not None, cmd

    cpu_time=None
    wall_time=None
    mem=None

    # returnvalue=0
    # walltime=0.0048832379980012774s
    # cputime=0.002279951s
    # memory=172032B

    if p.returncode == 0:
        for line in err.splitlines():
            if b"elapsed" in line:
                parts = line.split()
                assert b"user" in parts[0]
                assert b"elapsed" in parts[2]
                assert b"maxresident" in parts[5]

                try:
                    cpu_time = float(parts[0][:-4])
                    wall_time = float(parts[2][:-7])
                    mem = int(parts[5][:-13])/1024.0
                except ValueError as e:
                    print(err, file=sys.stderr)
                    raise e


    with lock:
        print(f"rvhyper{name_suffix}", traces_dir, traces_num, trace_len, bits, cpu_time, wall_time, mem, p.returncode)
        stdout.flush()
    #return (n, l, wbg_size, cpu_time, wall_time, mem)
    return 0



def run_hnl(arg, traces_dir, files):
    traces_num, trace_len, bits = arg
    cmd = [join(f"{hnl_dir}-{bits}b", "monitor")]
    cmd += files
    #print(cmd)
    p = Popen(RUNEXEC + cmd, stderr=PIPE, stdout=PIPE, cwd=traces_dir, preexec_fn=os.setsid)
    try:
        out, err = p.communicate(timeout=TIMEOUT)
    except TimeoutExpired:
        os.killpg(os.getpgid(p.pid), signal.SIGTERM) 
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
        print("hnl", traces_dir, traces_num, trace_len, bits, verdict, instances, atoms, reused_mons, reused_verdicts, cpu_time, wall_time, mem, p.returncode)
        stdout.flush()
    #return (n, l, wbg_size, cpu_time, wall_time, mem)

def run_mpt(arg, traces_dir, files):
    traces_num, trace_len, bits = arg
    cmd = [mpt_binary]
    cmd += files
    #print(cmd)
    p = Popen(RUNEXEC + cmd, stderr=PIPE, stdout=PIPE, cwd=traces_dir, preexec_fn=os.setsid)
    try:
        out, err = p.communicate(timeout=TIMEOUT)
    except TimeoutExpired:
        os.killpg(os.getpgid(p.pid), signal.SIGTERM) 
       #p.terminate()
       #p.kill()
        out, err = p.communicate(timeout=10)
    #assert p.returncode == 0, p
    # assert out is not None, cmd
    assert err is not None, cmd


    # Max workbag size: 7391
    #Traces #: 500
    #1.73user 0.03system 0:01.76elapsed 99%CPU (0avgtext+0avgdata 137604maxresident)k
    #0inputs+0outputs (0major+34642minor)pagefaults 0swaps
    wbg_size=None
    cpu_time=None
    wall_time=None
    mem=None
    print(err)
    print(out)
    if p.returncode in (0, 1):
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
        print("mpt", traces_dir, traces_num, trace_len, bits, wbg_size, cpu_time, wall_time, mem, p.returncode)
        stdout.flush()
    #return (n, l, wbg_size, cpu_time, wall_time, mem)



def get_params():
    for N in TRACES_NUMS:
    	for L in TRACES_LEN:
    	    for B in BITS:
                yield N, L, B

def run(args):
    print(f"\033[1;34mRunning using {args.j} workers\033[0m", file=stderr)

    if REPEAT_ONE_TRACE:
        num = 1
    else:
        num = TRACES_NUMS[-1]

    with Pool(processes=args.j) as pool:
        result = pool.map(run_one, get_params())

parser = argparse.ArgumentParser()
parser.add_argument("--1t", action='store_true')
parser.add_argument("-j", metavar="PROC_NUM", action='store', type=int)
#parser.add_argument("--traces-dir", help="Take traces from this dir. If the dir does not exists, generate traces to this dir", action='store')
args = parser.parse_args()
#if args.traces_dir:
#    args.traces_dir = abspath(args.traces_dir)

run(args)
