#!/usr/bin/env python

# This script finds the best two pairs of resistors for making a diff amp with
# a gain of 1.
#
# List all of your resistor values in a file (one per line), for example:
#
# 9.990
# 10.004
# 9.999
# 9.865
#
# Call this script with the name of that file as the first command-line
# argument.
#
#
#
# Background:
# -----------
#
# When creating a single-opamp difference amplifier (for subtracting out a
# voltage), it becomes very important to match the ratios of the resistor
# pairs.
#
#  v1---R1---+---Rf---+
#            |        |
#            |   |\   |
#            +---|-\  |
#                |  >---+----vout
#            +---|+/
#            |   |/
#            |
#  v2---R2---+---Rg---+
#                     |
#                    GND
#
# Specifically, the ratio of R1/Rf must very closely match R2/Rg.
#
# 1% metal film resistors are cheap and readily available, but 1% tolerance
# just isn't good enough to produce a good difference amplifier.
#
# Precision resistors (0.1% or better) are available, but are expensive.
#
# Luckily, with a high resolution multimeter (e.g. the UNI-T UT61E, $50)
# and a little bit of combinatorics, we should be able to find two pairs
# of 1% resistors within any group of 10 or so which form ratios which are
# very closely matched.
#
# As an example, I grabbed 10 random resistors out of my junk bin:
#
# 10.004
# 9.990
# 9.999
# 9.865
# 9.953
# 9.869
# 9.889
# 9.917
# 9.881
# 9.922
#
# Running this script prints the following output:
#
# $ ./best-four-of-n.py 10k-resistors.txt 
# Best result:
# Pair 1: r1/r10 (10.004/9.922), ratio: 1.00826446281
# Pair 2: r3/r8 (9.999/9.917), ratio: 1.00826862963
# These pairs form ratios which are mis-matched by 0.000413%

import sys

def usage(fd):
    fd.write("Usage: %s <resistors file>\n" % sys.argv[0])

if len(sys.argv) < 2:
    usage(sys.stderr)
    sys.exit(2)

resistors_filename = sys.argv[1]

with open(resistors_filename) as fd:
    lines = [line.rstrip() for line in fd.readlines()]
    resistors = []
    for (index, line) in enumerate(lines):
        line_num = index + 1
        if len(line) > 0 and line[0] != '#':
            resistor = float(line)
            resistors.append((line_num, resistor))

if len(resistors) < 4:
    sys.stderr.write("Error: need at least 4 resistors to make 2 pairs.\n")
    sys.exit(3)

import itertools
import pprint

groups_of_4 = itertools.combinations(resistors, 4)

ratio_ratios = []
for (a,b,c,d) in groups_of_4:
    paired_pairs = [
        ((a,b), (c,d)),
        ((b,a), (c,d)),

        ((a,c), (b,d)),
        ((c,a), (b,d)),

        ((a,d), (b,c)),
        ((d,a), (b,c)),
    ]
    for ((a,b),(c,d)) in paired_pairs:
        (i_a, r_a) = a
        (i_b, r_b) = b
        (i_d, r_c) = c
        (i_c, r_d) = d
        ratio_a = r_a/r_b
        ratio_b = r_c/r_d
        ratio_ratio = max(ratio_a, ratio_b) / min(ratio_a, ratio_b)
        ratio_ratios.append((ratio_ratio, ((a,b),(c,d))))

ratio_ratios.sort()

(ratio, (((i_a,r_a),(i_b,r_b)), ((i_c,r_c),(i_d,r_d)))) = ratio_ratios[0]
ratio_1 = r_a / r_b
ratio_2 = r_c / r_d
print "Best result:"
print "Pair 1: r%s/r%s (%s/%s), ratio: %s" % (i_a, i_b, r_a, r_b, ratio_1)
print "Pair 2: r%s/r%s (%s/%s), ratio: %s" % (i_c, i_d, r_c, r_d, ratio_2)
match_percent = abs(ratio_1 - ratio_2) / max(ratio_1, ratio_2) * 100
print "These pairs form ratios which are mis-matched by %f%%" % match_percent
