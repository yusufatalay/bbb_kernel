#ifndef IOCTL_TEST_H
#define IOCTL_TEST_H

struct mystruct{
	int repeat;
	char name[64];
};

#define WR_VALUE	_IOW('a', 1, int32_t *)
#define RD_VALUE	_IOR('a', 2, int32_t *)
#define GREETER	  	_IOR('a', 3, struct mystruct *)

#endif
