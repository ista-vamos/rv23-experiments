#/usr/bin/env python3
from random import randint, seed
import sys

def _encode_n(name, n):
    bitstr = str(bin(n))[2:][::-1]
    ns = [n for n, b in enumerate(bitstr) if b == "1"]
    return ",".join((f"{name}_{n}" for n in ns))

class IOEvent:
    """
    I/O Event
    """
    def __init__(self, n_in, n_out):
        self.n_in = n_in
        self.n_out = n_out

    def encode(self):
        return f"{_encode_n('in', self.n_in)};{_encode_n('out', self.n_out)}"

    def short_str(self):
        return f"{self.n_in};{self.n_out}"


p = 0.1
#p_err = 0

if len(sys.argv) < 4:
    print("Need arguments: <number of traces> <length of traces> <bits (number of atomic props.)>", file=sys.stderr)
    print(
f"""
The script will generate <number of traces> of length <length of traces>
Traces will contain `in` events with probability p={p} and a dummy (false) events
with complementary probability 1-p. The `in` event has atomic propositions
in_0, in_1, ... in_(<bits>-1).
They are populated from uniform range `(0, 2**<bits>-1)`, i.e., together `in`
is a random unsigned number on `BITS` bits.
The last event is an event that has also randomly set `out_*` bits.

Parameter `p` can be set in the script.
""", file=sys.stderr)
    exit(1)


TRACE_NUM = int(sys.argv[1])
TRACE_LEN = int(sys.argv[2])
BITS = int(sys.argv[3])
FORCE_OD = False
NO_STUTTERING = False
OUTDIR="."
if len(sys.argv) == 5:
    params = sys.argv[4].split(",")
    for param in params:
        param = param.strip()
        if "force-od" == param:
            FORCE_OD = True
        elif "no-stuttering" == param:
            NO_STUTTERING = True
        elif param.startswith("outdir="):
            OUTDIR=param.split("=")[1]
        else:
            raise NotImplementedError(f"Unknown parameter: {param}")

maxnum = (2**BITS) - 1

def gen_rand_in_event():
    """
    Generate I/O event with probability `p` and a dummy event with
    probability `1-p`. The I/O event will have random fields
    populated from uniform range `(0, 2**BITS-1)`, i.e., it is a random
    unsigned number on `BITS` bits.
    The dummy event is just all `in` and `out` bits set to 0.
    """
    p_0 = randint(0, TRACE_LEN)/TRACE_LEN
    if p_0 <= p:
        n_in = randint(0, maxnum)
        return IOEvent(n_in, 0)
    else:
        return IOEvent(0, 0)

seed() # initialize random numbers

#RESOLUTION = 1000

trnum = 0
gen_traces = {}
while trnum < TRACE_NUM:
    trnum += 1

    t_tmp = []
    for n in range(0, TRACE_LEN):
        e1 = gen_rand_in_event()

        if NO_STUTTERING and t_tmp:
            while e1 == t_tmp[-1]:
                e1 = gen_rand_in_event()

        # generate the output event if this is the last event
        # in the last event
        if n == TRACE_LEN - 1:
            e1 = IOEvent(0, randint(0, maxnum))
           #if randint(0, RESOLUTION) < (p_err * RESOLUTION):
           #    e2 = IOEvent(e1.n_in,  e1.n_out ^ 0x1)
           #    differ += 1

        t_tmp.append(e1)

    if FORCE_OD:
        h1 = " ".join((e.short_str() for e in t_tmp[:-1]))
        #print(h1)
        h1 = hash(h1)
        # if we have found a trace with the same input,
        # force the same output
        if h1 in gen_traces:
            # regenerate the traces
            t_tmp[-1] = gen_traces[h1]
           #print("FORCED to")
           #h1 = " ".join((e.short_str() for e in t_tmp[:-1]))
           #print(h1)
        else:
            gen_traces[h1] = t_tmp[-1]

    with open(f"{OUTDIR}/{trnum}.tr", "w") as tf:
        for e1 in t_tmp:
            print(e1.encode(), file=tf)

# DUMP OD PROPERTY ON THE GIVEN NUMBER OF BYTES
with open(f"{OUTDIR}/od-{BITS}b.hltl", "w") as f:
    f.write("forall x. forall y.\n(\n")
    for b in range(0, BITS):
        f.write(" & " if b > 0 else "   ")
        f.write(f"(out_{b}_x <-> out_{b}_y)\n")
    f.write(")\nW\n~(\n")
    for b in range(0, BITS):
        f.write(" & " if b > 0 else "   ")
        f.write(f"(in_{b}_x <-> in_{b}_y)\n")
    f.write(")\n")

print(f"Output dir: {OUTDIR}")
print(f"Forced OD: {FORCE_OD}")
print(f"No stuttering: {NO_STUTTERING}")
print(f"Generated {trnum} traces", file=sys.stderr)

exit(0)
