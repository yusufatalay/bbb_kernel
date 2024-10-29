#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yusuf Taha Atalay");
MODULE_DESCRIPTION("Registers a device number and implements some callback functions");

/**
 * @brief This function is called, when the device file is opened
 */
static int driver_open(struct inode *device_file, struct file *instance){
	printk("device_number - open was called!\n");
	return 0;
}

/**
 * @brief This function is called, when the device file is closed
 */
static int driver_close(struct inode *device_file, struct file *instance){
	printk("device_number - close was called!\n");
	return 0;
}

static struct file_operations fops={
	.owner = THIS_MODULE,
	.open = driver_open,
	.release = driver_close
};

#define MY_MAJOR 91

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init ModuleInit(void){
	int retval;
	printk("device_number - Hello, Kernel!\n");

	/* register device number */
	retval = register_chrdev(MY_MAJOR, "dev_number", &fops);
	if (retval == 0){
		printk("device_number - registered device number Major: %d, Minor: %d\n",MY_MAJOR, 0);
	}else if (retval > 0){
		printk("device_number - registered device number Major: %d, Minor: %d\n",retval >> 20, retval&0xffff);
	}else{
		printk("device_number - could not register device number!\n");
		return -1;
	}
	        	
	return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit ModuleExit(void){
	unregister_chrdev(MY_MAJOR, "dev_number");
	printk("device_number - Goodbye, Kernel!\n");

}

module_init(ModuleInit);
module_exit(ModuleExit);

