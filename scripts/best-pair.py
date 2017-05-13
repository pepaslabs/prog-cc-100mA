#!/usr/bin/env python

# Find the best pair of resistors which most closely matches the given ratio.
# This script is used to find the optimal resistors for a voltage divider.
#
# List your resistor values in a file, e.g. resistors.txt:
#
#   $ cat resistors.txt:
#   99.76
#   99.45
#   100.25
#   99.45
#   (etc)
#
# Then run this script with a desired divider ratio.  For example:
#
#   $ ./best-pair.py 1.0 resistors.txt
#
# The script will crunch some numbers and then spit out some results, e.g:
#
#   Best 3 pairs:
#   r4 & r6 (99.45 & 99.45): ratio = 1.000000 (error = 0.000000%)
#   r3 & r12 (99.76 & 99.77): ratio = 0.999900 (error = -0.010023%)
#   r8 & r17 (99.56 & 99.58): ratio = 0.999799 (error = -0.020084%)
#
# (It isn't really necessary to show the best three pairs -- typically you
# are only interested in the best pair.  This is done just to verify that the
# results "look right").

import sys

def usage(fd):
	fd.write("Usage: %s <target ratio> <resistors file>\n" % sys.argv[0])

if len(sys.argv) < 3:
	usage(sys.stderr)
	sys.exit(2)

target_ratio = float(sys.argv[1])
resistors_filename = sys.argv[2]

with open(resistors_filename) as fd:
	lines = [line.rstrip() for line in fd.readlines()]
	resistors = []
	for line in lines:
		if len(line) > 0 and line[0] != '#':
			resistor = float(line)
			resistors.append(resistor)

if len(resistors) < 2:
	sys.stderr.write("Error: need at least 2 resistors to make a pair.\n")
	sys.exit(3)

named_resistors = []
for i in range(len(resistors)):
	name = "r%s" % (i + 1)
	value = resistors[i]
	tup = (value, name)
	named_resistors.append(tup)

import itertools

pairs = itertools.combinations(named_resistors, 2)

ratios = []
for (a_tuple, b_tuple) in pairs:
	a_value, a_name = a_tuple
	b_value, b_name = b_tuple
	ratio = float(a_value) / b_value
	error = (ratio - target_ratio) / target_ratio
	sort_key = abs(error)
	ratios.append((sort_key, error, ratio, a_tuple, b_tuple))

ratios.sort()

n = 3
print "Best %s pairs:" % n
for i in range(n):
	(sort_key, error, ratio, (a_value, a_name), (b_value, b_name)) = ratios[i]
	print "%s & %s (%s & %s): ratio = %f (error = %f%%)" % (a_name, b_name, a_value, b_value, ratio, error*100.0)
