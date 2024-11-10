#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>


#include "ioctl_test.h"

/**
 * NOTE: To run this code, the kernel module in this file needed to be compiled and installed
 * and also a corresponding device file needed to be created for this case the command would be
 *
 * sudo mknod /dev/ioctl_example c 91 0
 *
 */

int main(void){
	// device spesific data
	int answer;
	struct mystruct test = { 4, "Yusuf"};

	int dev = open("/dev/ioctl_example", O_RDONLY);
	if (dev == -1){
		perror("open\n");
		return -1;
	}
	printf("device file opened successfully!\n");

	ioctl(dev, RD_VALUE, &answer);
	printf("The answer is %d\n", answer);

	answer = 26;
	
	ioctl(dev, WR_VALUE, &answer);
	ioctl(dev, RD_VALUE, &answer);
	printf("The answer is changed to %d\n", answer);

	ioctl(dev, GREETER, &test);


	close(dev);
	return 0;
}
