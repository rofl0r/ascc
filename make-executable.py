#!/usr/bin/python2
import sys, os, re

outn = "executable.s"
outf = open(outn, "w")
f = open("stdlib/stdlib.s", "r")
outf.write(f.read())

fn = sys.argv[1]
f = open(fn, "r")

func = ''
funcn = ''
while 1:
	s = f.readline()
	if s == '': break
	s = s.rstrip("\n")
	if s == ".text": continue
	if s.startswith('.'): break
	m = re.match(r"([a-z]+)\$[0-9]:.*", s)
	if m:
		if funcn not in ['',
			'memcpy',
			'strlen',
			'strcpy',
			'print',
			'puts']: outf.write(func)

		func = ''
		funcn = m.groups(0)[0]
		# strip off $N suffix, as main can be called with 0, 2 or 3 args
		if funcn == 'main':
			func = 'main:\n'
		else:
			func = s + '\n'
	else:
		func += s + '\n'

outf.write(func)
