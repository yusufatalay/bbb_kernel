#include <linux/module.h>
#include <linux/init.h>
#include <linux/serdev.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yusuf Taha Atalay");
MODULE_DESCRIPTION("A simple loopback driver for an UART port");

/* Declare the probe and remove functions */
static struct of_device_id serdev_echo_ids[] = {
	{
		.compatible = "brightlight,echodev",
	}, { /* sentinel */}
};

/*
 * @brief Callback is called whenenver a character is received
 */
static int serdev_echo_recv(struct serdev_device *serdev, const unsigned char *buffer, size_t size){
	printk("serial_device: Received %zu bytes  \"%s\"", size, buffer);
	return serdev_device_write_buf(serdev, buffer,size);
}

static const struct serdev_device_ops serdev_echo_ops = {
	.receive_buf = serdev_echo_recv,
};


/*
 * @brief This function is called on loading the driver
 */
static int serdev_echo_probe(struct serdev_device *serdev){
	int status;
	printk("serial_device: probing started\n");
	
	serdev_device_set_client_ops(serdev, &serdev_echo_ops);
	status = serdev_device_open(serdev);
	if (status){
		printk("serial_device: Error opening serial port!\n");
		return -status;
	}

	serdev_device_set_baudrate(serdev, 115200);
	serdev_device_set_flow_control(serdev, false);
	serdev_device_set_parity(serdev, SERDEV_PARITY_NONE);

	status = serdev_device_write_buf(serdev, "Type something: ",sizeof("Type something: "));
	printk("serial_device: Wrote %d bytes.\n", status);

	return 0;
}

/*
 * @brief This function is called on unloading the driver
 */
static void serdev_echo_remove(struct serdev_device *serdev){
	printk("serial_device: unloading the driver\n");
	serdev_device_close(serdev);

	return; 
}

MODULE_DEVICE_TABLE(of, serdev_echo_ids);

static struct serdev_device_driver serdev_echo_driver = {
	.probe = serdev_echo_probe,
	.remove = serdev_echo_remove,
	.driver = {
		.name = "serdev_echo",
		.of_match_table = serdev_echo_ids,
	},
};

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init ModuleInit(void){
	printk("serial_device: loading the driver...\n");
	if(serdev_device_driver_register(&serdev_echo_driver)){
		printk("serial_device: Error! Could not load driver\n");
		return -1;
	}
	return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit ModuleExit(void){
	printk("serial_device: unloading the driver...\n");
	serdev_device_driver_unregister(&serdev_echo_driver);
}

module_init(ModuleInit);
module_exit(ModuleExit);

