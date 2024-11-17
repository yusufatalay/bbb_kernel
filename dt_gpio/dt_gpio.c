#include <linux/module.h>
#include <linux/init.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/proc_fs.h>
#include <linux/gpio/consumer.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yusuf Taha Atalay");
MODULE_DESCRIPTION("An example GPIO Driver LKM");

/* GPIO variable */
static struct gpio_desc *my_led = NULL;

/* Global variables for procfs file */
static struct proc_dir_entry *proc_file;

/*
 * @brief Write data to buffer
 */
static ssize_t my_write(struct file *File, const char __user *user_buffer, size_t count, loff_t *offs) {
	char buf[3]; // Buffer to handle '0'/'1' + '\n' + '\0'
	int ret;

	// Ensure the count is not larger than the buffer size
	if (count > sizeof(buf) - 1) {
		printk("dt_gpio: Input size too large\n");
		return -EINVAL;
	}

	// Copy data from user space to kernel space
	ret = copy_from_user(buf, user_buffer, count);
	if (ret) {
		printk("dt_gpio: Failed to copy data from user space\n");
		return -EFAULT;
	}

	// Null-terminate the buffer for safety
	buf[count] = '\0';

	// Remove the newline character, if present
	if (buf[count - 1] == '\n') {
		buf[count - 1] = '\0';
	}

	// Parse the input and toggle the GPIO accordingly
	if (strcmp(buf, "0") == 0) {
		gpiod_set_value(my_led, 0);
	} else if (strcmp(buf, "1") == 0) {
		gpiod_set_value(my_led, 1);
	} else {
		printk("dt_gpio: Invalid input value. Use '0' or '1'\n");
		return -EINVAL;
	}

	return count;
}


static struct proc_ops fops = {
	.proc_write = my_write,
};

/* Declare the probe and remove functions */
static struct of_device_id my_driver_ids[] = {
	{
		.compatible = "brightlight,mydev",
	}, { /* sentinel */}

};

/*
 * @brief This function is called on loading the driver
 */
static int dt_probe(struct platform_device *pdev){
	struct device *dev = &pdev->dev;
	const char *label;
	int my_value, ret;

	printk("dt_gpio: probing started\n");

	/* Check for device properties */
	if(!device_property_present(dev, "label")){
		printk("dt_gpio: Error!  Device property 'label' not found\n");
		return -1;
	}

	if(!device_property_present(dev, "my_value")){
		printk("dt_gpio: Error!  Device property 'my_value' not found\n");
		return -1;
	}

	if(!device_property_present(dev, "orange-led-gpio")){
		printk("dt_gpio: Error!  Device property 'orange-led-gpio' not found\n");
		return -1;
	}

	/* Read device properties */
	ret = device_property_read_string(dev, "label", &label);
	if (ret){
		printk("dt_gpio: Error!  Could not read 'label'\n");
		return -1;

	}
	printk("dt_gpio: label: %s\n", label);

	ret = device_property_read_u32(dev, "my_value", &my_value);
	if (ret){
		printk("dt_gpio: Error!  Could not read 'my_value'\n");
		return -1;

	}
	printk("dt_gpio: my_value: %d\n", my_value);

	/* Init GPIO */
	my_led = gpiod_get(dev, "orange-led", GPIOD_OUT_LOW);
	if (IS_ERR(my_led)){
		printk("dt_gpio: Error!  Could not setup the GPIO\n");
		return -1 * IS_ERR(my_led);
	}

	/* Creating procfs file */
	proc_file = proc_create("my_led", 0666,NULL , &fops);
	if(proc_file == NULL){
		printk("dt_gpio: FAILED TO CREATE FILE  /proc/my_led\n");
		gpiod_put(my_led);
		return -ENOMEM;
	}	

	return 0;
}

/*
 * @brief This function is called on unloading the driver
 */
static int dt_remove(struct platform_device *pdev){
	printk("dt_gpio: unloading the driver\n");
	gpiod_put(my_led);
	proc_remove(proc_file);

	return 0;
}

MODULE_DEVICE_TABLE(of, my_driver_ids);

static struct platform_driver my_driver = {
	.probe = dt_probe,
	.remove = dt_remove,
	.driver = {
		.name = "my_device_driver",
		.of_match_table = my_driver_ids,
	},
};

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init ModuleInit(void){
	printk("dt_gpio: loading the driver...\n");
	if(platform_driver_register(&my_driver)){
		printk("dt_gpio: Error! Could not load driver\n");
		return -1;
	}
	return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit ModuleExit(void){
	printk("dt_gpio: unloading the driver...\n");
	platform_driver_unregister(&my_driver);
}

module_init(ModuleInit);
module_exit(ModuleExit);

