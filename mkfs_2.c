#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include "param.h"

#define BSIZE 512

int main(int argc, char *argv[])
{
	unsigned char buf[BSIZE];
	int ret;
	int rdfd = open(argv[1], O_RDONLY);
	if (rdfd < 0) {
		perror("rdfd open");
		exit(1);
	}
	int wrfd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0644);
	if (wrfd < 0) {
		perror("wrfd open");
		exit(1);
	}

	//TODO implement here
	ftruncate(wrfd, (FSSIZE * 2) * BSIZE);

	for (int blockno = 0; blockno < FSSIZE; blockno++) {
		ret = pread(rdfd, buf, BSIZE, (BSIZE * blockno));
		if (ret == 0)
			break;
		if (ret != BSIZE) {
			perror("pread");
			exit(1);
		}
		ret = pwrite(wrfd, buf, BSIZE, (BSIZE * blockno));
		if (ret != BSIZE) {
			perror("pwrite1");
			exit(1);
		}
		ret = pwrite(wrfd, buf, BSIZE, (blockno + FSSIZE) * BSIZE);
		if (ret != BSIZE) {
			perror("pwrite2");
			exit(1);
		}
	}
	close(rdfd);
	close(wrfd);
}
