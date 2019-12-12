#!/bin/sh
for i in tests/*.c ; do
	printf "testing %s..." "$i"
	fn="${i%.*}"
	if ! ./ascc -i DEVNULL -S "$i" ; then
		echo FAIL
		continue
	fi
	python make-executable.py "$fn".s
	agssim -i executable.s > test.$$.tmp
	ret=$?
	eret=$(cat "$fn".ret)
	if test $ret != $eret ; then
		echo FAIL
	elif ! diff -u test.$$.tmp "$fn".expect >/dev/null ; then
		echo FAIL2
	else
		echo OK
	fi
	rm -f test.$$.tmp
done
