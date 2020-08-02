#!/bin/env python

import struct

blocksize = 512


def makeBlock(n):
	buf = 'helodata' + struct.pack('<I', n);

	# from "Tables of Linear Congruential Generators of Different Sizes and
	# Good Lattice Structure" - Peter L'Ecuyer, Mathematics of Computation,
	# Volume 68, Number 225, January 1999
	m = (1 << 32) - 5;
	a = 1815976680;

	x = n
	for j in xrange(0, (blocksize - len(buf))/4):
		x = (x * a) % m
		buf += struct.pack('<I', x);
	return buf

def writeFile():
	with open('data.dat', 'wb') as f:
		for i in xrange(0, 400):
			f.write(makeBlock(i))

if __name__ == "__main__":
	writeFile()
