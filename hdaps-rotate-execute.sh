#!/usr/bin/bash
#
# hdaps-rotate-execute - Execute command on rotate
#

ROTATE=$1

case "$1" in
	"left")
		ROTATE="right"
		;;
	"right")
		ROTATE="left"
		;;
esac

for DISPLAY in $(xrandr | sed -n '/connected/{s/^\([A-Z]*[0-9]*\) connected.*$/\1/p}'); do
	xrandr -d :0 --output ${DISPLAY} --rotate ${ROTATE}
done
