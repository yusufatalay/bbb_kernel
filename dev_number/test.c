#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

/**
 * NOTE: To run this code, the kernel module in this file needed to be compiled and installed
 * and also a corresponding device file needed to be created for this case the command would be
 *
 * sudo mknod /dev/mydevice c 90 0
 *
 */

int main(void){
	int dev = open("/dev/mydevice", O_RDONLY);
	if (dev == -1){
		perror("open\n");
		return -1;
	}
	printf("device file opened successfully!\n");

	close(dev);
	return 0;
}
