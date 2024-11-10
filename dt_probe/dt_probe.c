#include <linux/module.h>
#include <linux/init.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yusuf Taha Atalay");
MODULE_DESCRIPTION("An example LKM that parses device tree for a spesific device and its properties");

/* Declare the probe and remove functions */
static struct of_device_id my_driver_ids[] = {
	{
		.compatible = "brightlight, mydev",
	}, { /* sentinel */}

};

/*
 * @brief This function is called on loading the driver
 */
static int dt_probe(struct platform_device *pdev){
	struct device *dev = &pdev->dev;
	const char *label;
	int my_value, ret;

	printk("dt_probe: probing started\n");

	/* Check for device properties */
	if(!device_property_present(dev, "label")){
		printk("dt_probe: Error!  Device property 'label' not found\n");
		return -1;
	}

	if(!device_property_present(dev, "my_value")){
		printk("dt_probe: Error!  Device property 'my_value' not found\n");
		return -1;
	}

	/* Read device properties */
	ret = device_property_read_string(dev, "label", &label);
	if (ret){
		printk("dt_probe: Error!  Could not read 'label'\n");
		return -1;

	}
	printk("dt_probe: label: %s\n", label);

	ret = device_property_read_u32(dev, "my_value", &my_value);
	if (ret){
		printk("dt_probe: Error!  Could not read 'my_value'\n");
		return -1;

	}
	printk("dt_probe: my_value: %d\n", my_value);

	return 0;
}

/*
 * @brief This function is called on unloading the driver
 */
static int dt_remove(struct platform_device *pdev){
	printk("dt_probe: unloading the driver\n");

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
	printk("dt_probe: loading the driver...\n");
	if(platform_driver_register(&my_driver)){
		printk("dt_probe: Error! Could not load driver\n");
		return -1;
	}
	return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit ModuleExit(void){
	printk("dt_probe: unloading the driver...\n");
	platform_driver_unregister(&my_driver);
}

module_init(ModuleInit);
module_exit(ModuleExit);

