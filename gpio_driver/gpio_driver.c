#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yusuf Taha Atalay");
MODULE_DESCRIPTION("A simple gpio driver for setting a LED and reading a button");


/* Variables for device and device class */
static dev_t my_device_number;
static struct class *my_class;
static struct cdev my_device;

#define DRIVER_NAME   "my_gpio_driver"
#define DRIVER_CLASS  "MyModuleClass"

/**
 * @brief Read data out of the buffer
 */
static ssize_t driver_read(struct file *File, char *user_buffer, size_t count, loff_t *offs){
	int to_copy, not_copied, delta;
	char tmp[3] = " \n";

	/* Get amount of data to copy */
	to_copy = min(count, sizeof(tmp));

	/* Read value of button */
	printk("gpio_driver - Value of button: %d\n", gpio_get_value(48));
	tmp[0] = gpio_get_value(48) + '0';

	/* Copy data to user */
	not_copied = copy_to_user(user_buffer, &tmp, to_copy);

	/* Calculate data */
	delta = to_copy - not_copied;

	return delta;
}

/**
 * @brief Write data to buffer
 */
static ssize_t driver_write(struct file *File,const char *user_buffer, size_t count, loff_t *offs){
	int to_copy, not_copied, delta;
	char value;

	/* Get amount of data to copy */
	to_copy = min(count, sizeof(value));

	/* Copy data to user */
	not_copied = copy_from_user( &value, user_buffer, to_copy);

	printk("gpio_driver - User set the value: %d\n", value);
	/* Setting the LED */
	switch(value){

		case '0':
			gpio_set_value(60,0);
			break;
		case '1':	
			gpio_set_value(60,1);
			break;
		default:
			printk("gpio_driver - Invalid Input!\n");
			break;
	}

	/* Calculate data */
	delta = to_copy - not_copied;

	return delta;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int driver_open(struct inode *device_file, struct file *instance){
	printk("gpio_driver - open was called!\n");
	return 0;
}

/**
 * @brief This function is called, when the device file is closed
 */
static int driver_close(struct inode *device_file, struct file *instance){
	printk("gpio_driver - close was called!\n");
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
	printk("gpio_driver - Hello, Kernel!\n");

	/* Allocate a device number */
	if (alloc_chrdev_region(&my_device_number, 0 ,1, DRIVER_NAME) < 0){
		printk("gpio_driver - Device number could not be allocated!\n"); 
		return -1;
	}

	printk("gpio_driver - Device number Major: %d, Minor %d was registered!\n", my_device_number >> 20, my_device_number & 0xfffff);
	        	
	/* Create device class */
	if ((my_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL){
		printk("gpio_driver - Device class can not be created!\n");
		goto ClassError;
	}

	/* Create a device file  */
	if(device_create(my_class, NULL, my_device_number, NULL, DRIVER_NAME) == NULL){
		printk("gpio_driver - Device file can not be created!\n");
		goto FileError;
	}

	/* Initialize device file */
	cdev_init(&my_device, &fops);

	/* Registering device to kernel */
	if(cdev_add(&my_device, my_device_number, 1) < 0 ){
		printk("gpio_driver - Registering of device to kernel failed!\n");
		goto AddError;
	}

	/* GPIO P9_12 (LED) Init */
	if(gpio_request(60, "P9_12") != 0){
		printk("gpio_driver - Can not allocate GPIO 60\n");
		goto AddError;
	}

	/* Set GPIO direction */
	if(gpio_direction_output(60, 0) != 0){
		printk("gpio_driver - Can not set GPIO 60 as output!\n");
		goto GPIO60Error;
	}

	/* GPIO P9_15 (Button) Init */
	if(gpio_request(48, "P9_15") != 0){
		printk("gpio_driver - Can not allocate GPIO 48\n");
		goto GPIO60Error;
	}

	/* Set GPIO direction */
	if(gpio_direction_input(48) != 0){
		printk("gpio_driver - Can not set GPIO 48 as input!\n");
		goto GPIO48Error;
	}	
	return 0;


GPIO48Error:
	gpio_free(48);
GPIO60Error:
	gpio_free(60);
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
	gpio_set_value(60,0);
	gpio_free(48);
	gpio_free(60);
	cdev_del(&my_device);
	device_destroy(my_class, my_device_number);
	class_destroy(my_class);
	unregister_chrdev(my_device_number, DRIVER_NAME);

	printk("gpio_driver - Goodbye, Kernel!\n");

}

module_init(ModuleInit);
module_exit(ModuleExit);

