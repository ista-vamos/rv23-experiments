#!/usr/bin/env python3

from tempfile import mkdtemp
from subprocess import Popen, PIPE, DEVNULL, run as runcmd, TimeoutExpired
from os.path import dirname, realpath, basename, join, isfile
from os import listdir, access, X_OK, environ as ENV, symlink
from sys import argv, stderr
from multiprocessing import Pool, Lock

lock = Lock()

bindir = f"{dirname(realpath(__file__))}/"
binary = join(bindir, "monitor-rvhyper")
rvhyper_dir = join(bindir, "rvhyper")

TIMEOUT = 120
# Do not generate `traces_num` random traces, but generate just one
# and use it `traces_num` times (this will force the monitors to
# read entire traces to the end)
REPEAT_ONE_TRACE = False

def errlog(*args):
    with open(join(dirname(__file__), "log.txt"), "a") as logf:
        for a in args:
            print(a, file=logf)

def run_one(arg):
    traces_dir, traces_num, trace_len, bits = arg

    # get the list of files
    if REPEAT_ONE_TRACE:
        files = ["1.tr"] * traces_num
    else:
        files = []
        for fl in listdir(traces_dir):
            if fl.endswith(".tr"):
                files.append(fl)
                if len(files) == traces_num:
                    break

    # run our monitor
    run_monitor(arg, files)

    # run rvhyper
    run_rvhyper(arg, files)

    # run rvhyper with --sequential
    run_rvhyper(arg, files, ["--sequential"], "-seq")


def run_rvhyper(arg, files, rvh_args=None, name_suffix=""):
    traces_dir, traces_num, trace_len, bits = arg
    rvh = join(rvhyper_dir, "build/release/rvhyper")
    assert access(rvh, X_OK), f"Cannon find rvhyper binary, assumed is {rvh}"
    cmd = ["/bin/time", "-f", '%Uuser %Ssystem %eelapsed %PCPU (%Xavgtext+%Davgdata %Mmaxresident)k',
           rvh, "--quiet"]
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
    env["LD_LIBRARY_PATH"] = join(rvhyper_dir, "lib")
    p = Popen(cmd, stderr=PIPE, stdout=PIPE, cwd=traces_dir, env=env)
    try:
        out, err = p.communicate(timeout=TIMEOUT)
        if p.returncode != 0:
            errlog(env, p, out, err)
    except TimeoutExpired:
        p.kill()
        out, err = p.communicate()
    #print(p, p.returncode, out, err)
    #assert p.returncode == 0, p
    assert err is not None, cmd
    #assert out is not None, cmd

    # Max workbag size: 7391
    #Traces #: 500
    #1.73user 0.03system 01.76elapsed 99%CPU (0avgtext+0avgdata 137604maxresident)k
    wbg_size=None
    cpu_time=None
    wall_time=None
    mem=None

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
        print(f"rvhyper{name_suffix}", traces_num, trace_len, bits, cpu_time, wall_time, mem, p.returncode)
    #return (n, l, wbg_size, cpu_time, wall_time, mem)
    return 0



def run_monitor(arg, files):
    traces_dir, traces_num, trace_len, bits = arg
    cmd = ["/bin/time", "-f", '%Uuser %Ssystem %eelapsed %PCPU (%Xavgtext+%Davgdata %Mmaxresident)k',
           binary]
    cmd += files
    #print(cmd)
    p = Popen(cmd, stderr=PIPE, stdout=PIPE, cwd=traces_dir)
    try:
        out, err = p.communicate(timeout=TIMEOUT)
    except TimeoutExpired:
        p.kill()
        out, err = p.communicate()
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
        print("mpt", traces_num, trace_len, bits, wbg_size, cpu_time, wall_time, mem, p.returncode)
    #return (n, l, wbg_size, cpu_time, wall_time, mem)

traces_num = [500, 1000, 1500, 2000, 2500, 3000]
traces_num = [5, 10, 15]

def get_params(traces_dir, trace_len, bits):
    for N in traces_num:
        yield traces_dir, N, trace_len, bits

def run(trace_len, bits, proc_num):
    print(f"\033[1;34mRunning trace_len={trace_len}, bits={bits} [using {proc_num} workers]\033[0m", file=stderr)

    if REPEAT_ONE_TRACE:
        num = 1
    else:
        num = traces_num[-1]

    # generate all traces and then always take just some of them
    traces_dir = mkdtemp(prefix="/tmp/")
    runcmd(["python", f"{bindir}/gen-traces.py",
            str(num), str(trace_len), str(bits), f"force-od,outdir={traces_dir}"],
           stderr=DEVNULL, stdout=DEVNULL, check=True)

    with Pool(processes=proc_num) as pool:
        result = pool.map(run_one, get_params(traces_dir, trace_len, bits))

proc_num=None
if len(argv) == 2:
    if argv[1] == "1t":
        REPEAT_ONE_TRACE = True
    else:
        proc_num=int(argv[1])

if len(argv) == 3:
    if argv[1] == "1t":
        REPEAT_ONE_TRACE = True
        proc_num=int(argv[2])
    elif argv[2] == "1t":
        REPEAT_ONE_TRACE = True
        proc_num=int(argv[1])
    else:
        raise RuntimeError(f"Unknown arguments: {argv}")

for trace_len in (100, 500, 1000):
    for bits in (1, 2, 4, 8, 16, 32, 64):
        run(trace_len, bits, proc_num)
