# hdaps-rotate

Simple application that executes a script depending on the HDAPS accelerometer state on ThinkPads. Compile with `make`. Comes with a default script that rotates all connected displays.


```
Usage: ./hdaps-rotate [OPTIONS]
Options:
	-v, --verbose			Print out thresholds and measurements
	-l, --threshold-left=I		Threshold for rotating left
	-r, --threshold-right=I		Threshold for rotating right
	-i, --interval=i		Polling interval
	-e, --execute=SCRIPT		Script to execute on rotation. The script will be passed one parameter: left, right or normal
```