#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <X11/extensions/Xrandr.h>

#define	SYSFS_POSITION_FILE	"/sys/devices/platform/hdaps/position"
#define	BUF_LEN_SHORT		32
#define	BUF_LEN_LONG		4096
#define	THRESH_LEFT		-580
#define	THRESH_RIGHT		-380
#define	SLEEP_INTERVAL		500		// interval in milliseconds
#define	EXECUTE			"./hdaps-rotate-execute.sh"

typedef enum {
	LEFT,
	RIGHT,
	NORMAL
} orientation_t;

/*
 * read_position - read the (x,y) position pair from hdaps.
 *
 * We open and close the file on every invocation, which is lame but due to
 * several features of sysfs files:
 *
 *	(a) Sysfs files are seekable.
 *	(b) Seeking to zero and then rereading does not seem to work.
 *
 * If I were king--and I will be one day--I would have made sysfs files
 * nonseekable and only able to return full-size reads.
 */
static int read_position (int *x, int *y){
	char buf[BUF_LEN_SHORT];
	int fd, ret;

	fd = open(SYSFS_POSITION_FILE, O_RDONLY);
	if(fd < 0){
		perror("open " SYSFS_POSITION_FILE);
		return fd;
	}

	ret = read(fd, buf, BUF_LEN_SHORT);
	if(ret < 0){
		perror("read " SYSFS_POSITION_FILE);
		goto out;
	} else if(ret == 0){
		fprintf(stderr, "error: unexpectedly read zero!\n");
		ret = 1;
		goto out;
	}
	ret = 0;

	if(sscanf(buf, "(%d,%d)\n", x, y) != 2)
		ret = 1;

out:
	if(close(fd))
		perror ("close " SYSFS_POSITION_FILE);
	return ret;
}

int main (int argc, char** argv){
	// Check for hdaps module
	FILE* fd_mod;
	char line[BUF_LEN_LONG], mod[BUF_LEN_SHORT];

	fd_mod = fopen("/proc/modules", "r");
	if(!fd_mod){
		perror("open /proc/modules");
		exit(EXIT_FAILURE);
	}

	while(fgets(line, sizeof(line), fd_mod)){
		sscanf(line, "%s", mod);
		if(strcmp(mod, "hdaps") == 0)
			break;
	}
	fclose(fd_mod);
	if(strcmp(mod, "hdaps") != 0){
		fprintf(stderr, "Please load the hdaps module first:\n\n# modprobe hdaps\n\n");
		exit(EXIT_FAILURE);
	}


	static int verbose;
	int thresh_left = THRESH_LEFT;
	int thresh_right = THRESH_RIGHT;
	int sleep_interval = SLEEP_INTERVAL;
	char *exec = EXECUTE;

	int c;

	while (1){
		static struct option long_options[] = {
			{"verbose",		no_argument,		&verbose, 'v'},
			{"threshold-left",	required_argument,	0, 'l'},
			{"threshold-right",	required_argument,	0, 'r'},
			{"interval",		required_argument,	0, 'i'},
			{"execute",		required_argument,	0, 'e'},
			{0, 0, 0, 0}
		};
		int option_index = 0;
 
		c = getopt_long(argc, argv, "vl:r:i:e:",
						long_options, &option_index);
 
		if(c == -1) break;
 
		switch (c){
			case 0:
				break;
			case 'v':
				verbose = 1;
				break;
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
			case 'e':
				printf("Execute command is now %s\n", optarg);
				exec = optarg;
				break;
			default:
				printf("Usage: %s [OPTIONS]\n", argv[0]);
				printf("Options:\n");
				printf("\t-v, --verbose\t\t\tPrint out threshold and measurements\n");
				printf("\t-l, --threshold-left=I\t\tThreshold for rotating left\n");
				printf("\t-r, --threshold-right=I\t\tThreshold for rotating right\n");
				printf("\t-i, --interval=i\t\tPolling interval\n");
				printf("\t-e, --execute=SCRIPT\t\tScript to execute on rotation. The script will be passed one parameter: left, right or normal\n");
				exit(EXIT_FAILURE);
		}
	}

	int ret, x, y;
	orientation_t rot = NORMAL;
	char buf[256];

	if(verbose) printf("Left threshold\tMeasurement\tRight threshold\n");

	/* main loop */
	while(1){
		ret = read_position(&x, &y);
		if(ret < 0)
			return 1;

		if(x <= thresh_left){
			if(rot != LEFT){
				rot = LEFT;
				snprintf(buf, 256, "%s %s", exec, "left");
				system(buf);
			}
		} else if(x >= thresh_right){
			if(rot != RIGHT){
				rot = RIGHT;
				snprintf(buf, 256, "%s %s", exec, "right");
				system(buf);
			}
		} else {
			if(rot != NORMAL){
				rot = NORMAL;
				snprintf(buf, 256, "%s %s", exec, "normal");
				system(buf);
			}
		}

		if(verbose){
			switch(rot){
				case LEFT:
					printf("[%i]\t%i\t%i\n", THRESH_LEFT, x, THRESH_RIGHT);
					break;
				case RIGHT:
					printf("%i\t%i\t[%i]\n", THRESH_LEFT, x, THRESH_RIGHT);
					break;
				default:
					printf("%i\t[%i]\t%i\n", THRESH_LEFT, x, THRESH_RIGHT);
					break;
			}
		}

		nanosleep((struct timespec[]){{0, sleep_interval * 1000000}}, NULL);
	}

	exit(EXIT_SUCCESS);
}
