#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <X11/extensions/Xrandr.h>

#define SLEEP_INTERVAL		1	/* seconds */
#define SYSFS_POSITION_FILE	"/sys/devices/platform/hdaps/position"
#define BUF_LEN			32
#define THRESH_LEFT		-300
#define THRESH_RIGHT		300

/*
 * read_position - read the (x,y) position pair from hdaps.
 *
 * We open and close the file on every invocation, which is lame but due to
 * several features of sysfs files:
 *
 *	(a) Sysfs files are seekable.
 *	(b) Seeking to zero and then rereading does not seem to work.
 *
 * If I were king--and I will be one day--I would of made sysfs files
 * nonseekable and only able to return full-size reads.
 */
static int read_position (int *x, int *y)
{
	char buf[BUF_LEN];
	int fd, ret;

	fd = open (SYSFS_POSITION_FILE, O_RDONLY);
	if (fd < 0) {
		perror ("open");
		return fd;
	}	

	ret = read (fd, buf, BUF_LEN);
	if (ret < 0) {
		perror ("read");
		goto out;
	} else if (ret == 0) {
		fprintf (stderr, "error: unexpectedly read zero!\n");
		ret = 1;
		goto out;
	}
	ret = 0;

	if (sscanf (buf, "(%d,%d)\n", x, y) != 2)
		ret = 1;

out:
	if (close (fd))
		perror ("close");

	return ret;
}

int main (int argc, char** argv)
{
	int thresh_left = -580;
	int thresh_right = -380;
	int sleep_interval = 3;

	int c;

	while (1)
	{
		static struct option long_options[] =
		{
			{"threshold-left",	required_argument, 0, 'l'},
			{"threshold-right",	required_argument, 0, 'r'},
			{"interval",		required_argument, 0, 'i'},
			{0, 0, 0, 0}
		};
		int option_index = 0;
 
		c = getopt_long(argc, argv, "l:r:i:",
						long_options, &option_index);
 
		if (c == -1) break;
 
		switch (c)
		{
		case 'r':
			printf ("Threshold right is now %s\n", optarg);
			thresh_right = atoi(optarg);
			break;
 
		case 'l':
			printf ("Threshold left is now %s\n", optarg);
			thresh_left = atoi(optarg);
			break; 

		case 'i':
			printf ("Sleeping interval is now %s\n", optarg);
			sleep_interval = atoi(optarg);
			break;

		 default:
			abort();
		}
	}

	int ret, x, y;

	/* main loop */
	while(1)
	{
		ret = read_position(&x, &y);
		if(ret)
			return 1;
		printf("Angle: (%i)\n", x);
		if(x <= thresh_left)
		{
			printf("ROTATING LEFT\n");
		} else if(x >= thresh_right)
		{
			printf("ROTATING RIGHT\n");
		}
		sleep(sleep_interval);
	}

	return 0;
}
