#!/bin/sh
for i in tests/*.c ; do
	printf "testing %s..." "$i"
	fn="${i%.*}"
	if ! ./ascc $ASCCFLAGS -P "cpp -P -I tests" -i DEVNULL -S "$i" ; then
		echo FAIL
		continue
	fi
	python make-executable.py "$fn".s
	optargs=
	test -f "$fn".args && optargs="-- $(cat "$fn".args)"
	agssim -i executable.s $optargs > test.$$.tmp
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
