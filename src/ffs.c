#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

/* see man feature_test_macros and man 2 open section on O_LARGEFILE.
   This is needed to write files larger than 2GB due to offset type,
   when running on 32-bit architectures. */
/* TODO: when writing makefile, use flag -D_FILE_OFFSET_BITS=64 to enable this stuff */
/* TODO: find a better ifdef. Which platforms have features.h? NetBSD apparently
   supports 64 bit offsets without any special features enabled. */
#ifdef __LINUX__
#include <features.h> 
#endif

#define BUFFSIZE	8192

extern int errno;

/* returns 1 on success (file size max reached, not file system max).
   0 on failure. May exit(1) on open() failure.*/
int write_file(char *name, char *data); /* requires null termination */

int main(int argc, char** argv)	 /* TODO filename base from argv */
{
	int	i = 0;
	int  spaceleft = 0;
	char namebase[] = "binzero"; /* TODO: check for name collisions, maybe here, prolly in write_file() */
	char filename[PATH_MAX];
	char wipeage[BUFFSIZE];
	memset(wipeage, 0, BUFFSIZE);
	
	printf("ffs initializing...\t\t\n");
	printf("ffs filling fs...\n");

	/* here, determine/construct filename and call write_file() */
	/* needs to be in a loop for coming up with filnames until write_file() returns false */
	do {
		snprintf(filename, PATH_MAX, "%s_%.5d", namebase, i++);
		printf("calling write_file() on %s\n", filename);
		spaceleft = write_file(filename, wipeage);
	} while (spaceleft != 0);


	/* TODO: Loop back over filenames and clean up */

	printf("ffs succeeded. or unknown error ;) XD\n");
	return 0;
}

int write_file(char *name, char *data)
{
	int r = 0;
	int fd = 0;
	ssize_t bytes = 0;
	fd = open(name, O_CREAT | O_WRONLY 
				| O_APPEND | O_EXCL);  /* TODO: umask */
													 /* TODO: handle name collision */
	/* TODO: if file exists, return true so we can try the next filename? */
	if (fd == -1) {
		if (errno == EEXIST) {
			perror("open() failed! ");
			fprintf(stdout, 
					"got EEXIST from open(), might try another name...\n");
			r = 1; /*file exists is nonfatal, allow caller to try another filename */
		} else {
			fprintf(stderr, "open() failed! ");
			printf ("Received unknown error (%s) from open()!\n", strerror(errno));
			fprintf(stdout, "got unknown errno from open(), bailing out...\n");
			exit(1); /* consider other open() errors fatal */
		}
	}
	else {
		do {
			bytes = write(fd, data, BUFFSIZE);
		} while (bytes != -1);
		/* TODO: invokation name here, need to copy or pass from main() */
		perror("write() failed (normal message if no space or file too big)");
		close(fd); 

		/* poor control flow style, just a hack program. */
		/* switch statement may be a bit cleaner */
		if ( (errno != EFBIG) && (errno != ENOSPC) ) {
			perror ("write() unknown error!\n");
			printf ("Bailing!\n");
			exit(1);
		}

		/* check for full filesystem vs file size max */
		if (errno == EFBIG) 
			r = 1;
		else
			r = 0; /* check for other errors? */

		/* assyme if we got here and r is 0, ENOSPC received. Other errors
		   should bail. */
		printf(". ");

	}	
	return r;
}
