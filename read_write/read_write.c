#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yusuf Taha Atalay");
MODULE_DESCRIPTION("Registers a  and implements some callback functions");

/* Buffer for data */
static char buffer[255];
static int buffer_pointer;

/* Variables for device and device class */
static dev_t my_device_number;
static struct class *my_class;
static struct cdev my_device;

#define DRIVER_NAME   "dummydriver"
#define DRIVER_CLASS  "MyModuleClass"

/**
 * @brief Read data out of the buffer
 */
static ssize_t driver_read(struct file *File, char *user_buffer, size_t count, loff_t *offs){
	int to_copy, not_copied, delta;

	/* Get amount of data to copy */
	to_copy = min(count, buffer_pointer);

	/* Copy data to user */
	not_copied = copy_to_user(user_buffer, buffer, to_copy);

	/* Calculate data */
	delta = to_copy - not_copied;

	return delta;
}

/**
 * @brief Write data to buffer
 */
static ssize_t driver_write(struct file *File,const char *user_buffer, size_t count, loff_t *offs){
	int to_copy, not_copied, delta;

	/* Get amount of data to copy */
	to_copy = min(count, sizeof(buffer));

	/* Copy data to user */
	not_copied = copy_from_user( buffer, user_buffer, to_copy);
	buffer_pointer = to_copy;

	/* Calculate data */
	delta = to_copy - not_copied;

	return delta;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int driver_open(struct inode *device_file, struct file *instance){
	printk("read_write - open was called!\n");
	return 0;
}

/**
 * @brief This function is called, when the device file is closed
 */
static int driver_close(struct inode *device_file, struct file *instance){
	printk("read_write - close was called!\n");
	return 0;
}

static struct file_operations fops={
	.owner = THIS_MODULE,
	.open = driver_open,
	.release = driver_close,
	.read = driver_read,
	.write = driver_write
};

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init ModuleInit(void){
	int retval;
	printk("read_write - Hello, Kernel!\n");

	/* Allocate a device number */
	if (alloc_chrdev_region(&my_device_number, 0 ,1, DRIVER_NAME) < 0){
		printk("read_write - Device number could not be allocated!\n"); 
		return -1;
	}

	printk("read_write - Device number Major: %d, Minor %d was registered!\n", my_device_number >> 20, my_device_number & 0xfffff);
	        	
	/* Create device class */
	if ((my_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL){
		printk("read_write - Device class can not be created!\n");
		goto ClassError;
	}

	/* Create a device file  */
	if(device_create(my_class, NULL, my_device_number, NULL, DRIVER_NAME) == NULL){
		printk("read_write - Device file can not be created!\n");
		goto FileError;
	}

	/* Initialize device file */
	cdev_init(&my_device, &fops);

	/* Registering device to kernel */
	if(cdev_add(&my_device, my_device_number, 1) < 0 ){
		printk("read_write - Registering of device to kernel failed!\n");
		goto AddError;
	}

	
	return 0;
AddError:
	device_destroy(my_class, my_device_number);
FileError:
	class_destroy(my_class);
ClassError:
	unregister_chrdev(my_device_number, DRIVER_NAME);
	return -1;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit ModuleExit(void){
	cdev_del(&my_device);
	device_destroy(my_class, my_device_number);
	class_destroy(my_class);
	unregister_chrdev(my_device_number, DRIVER_NAME);

	printk("read_write - Goodbye, Kernel!\n");

}

module_init(ModuleInit);
module_exit(ModuleExit);

