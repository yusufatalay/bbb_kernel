#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/kernel.h>

#define DRIVER_NAME   "bme280"
#define DRIVER_CLASS  "bme280Class"

static struct i2c_adapter * bme280_i2c_adapter = NULL;	/* Adapter to I2C Bus */
static struct i2c_client * bme280_i2c_client = NULL;	/* Client: oiut I2C device */

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yusuf Taha Atalay");
MODULE_DESCRIPTION("A driver for reading out a BME280 pressure, temperature  sensor");
MODULE_SUPPORTED_DEVICE("NONE");

/* Defines for device identification */
#define I2C_BUS_AVAILABLE	2		/* The I2C Bus available on the beaglebone black */
#define SLAVE_DEVICE_NAME	"BME280"	/* Device and Driver Name */	
#define BME280_SLAVE_ADDRESS	0x76		/* BME280 I2C address */

static const struct i2c_device_id bme280_id[]={
	{SLAVE_DEVICE_NAME, 0},
	{}
};

static struct i2c_driver bme280_driver = {
	.driver = {
		.name = SLAVE_DEVICE_NAME,
		.owner = THIS_MODULE
	}
};

static struct i2c_board_info bme280_i2c_board_info = {
	I2C_BOARD_INFO(SLAVE_DEVICE_NAME, BME280_SLAVE_ADDRESS)
};

/* Variables for device and device class */
static dev_t my_device_number;
static struct class *my_class;
static struct cdev my_device;

/* Variables for temperature calculation */
s32 dig_T1, dig_T2, dig_T3;
/* Variable for pressure calculation */
s32 dig_P1, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
s32 t_fine;

/**
 * @brief Read current temperature from BME280
 * return temperature in degree celcius
 */
s32 read_temperature(void){
	int var1, var2;
	s32 raw_temp;
	s32 d1,d2,d3;

	/* Read the temperature registers */
	d1 = i2c_smbus_read_word_data(bme280_i2c_client, 0xFA);
	d2 = i2c_smbus_read_word_data(bme280_i2c_client, 0xFB);
	d3 = i2c_smbus_read_word_data(bme280_i2c_client, 0xFC);
	raw_temp = ((d1 << 16) | (d2 << 8) | d3) >> 3;

	/* Calculate Temperature */
	var1 = ((((raw_temp >> 3) - (dig_T1 << 1))) * (dig_T2)) >> 11;

	var2 = (((((raw_temp >> 4) - (dig_T1)) * ((raw_temp >> 4) - (dig_T1))) >> 12) * (dig_T3)) >> 14;
	t_fine = var1 + var2;
	return (t_fine *5 +128) >> 8;
}

/**
 * @brief Read current pressure from BME280
 * return pressure in Pa 
 */
s32 read_pressure(void){
	int var1, var2;
	int p;
	s32 raw_pressure;
	s32 d1,d2,d3;

	/* Read the pressure registers */
	d1 = i2c_smbus_read_word_data(bme280_i2c_client, 0xF7);
	d2 = i2c_smbus_read_word_data(bme280_i2c_client, 0xF8);
	d3 = i2c_smbus_read_word_data(bme280_i2c_client, 0xF9);
	raw_pressure = ((d1 << 16) | (d2 << 8) | d3) >> 3;

	var1 = t_fine - 128000;
	var2 = var1*var1*dig_P6;
	var2 = var2 + ((var1 *dig_P5)<<17);
	var2 = var2 + ((dig_P4)<<35);
	var1 = ((var1*var1*dig_P3)>>8) + ((var1 + dig_P2) << 12);
	var1 = (((1<<47) + var1))*dig_P1 >>33;
	if (var1 == 0){
		return 0;	// avoid division by 0 exception
	}

	p = 1048576 - raw_pressure;
	p = (((p << 31) - var2) *3125)/var1;
	var1 = (dig_P9 *(p>>13) * (p>>13)) >> 25;
	var2 = (dig_P8 * p) >> 19;
	p = ((p + var1 + var1) >> 8) + (dig_P7 << 4);
	return (p/256) *100;
}


/**
 * @brief Read data out of the buffer
 */
static ssize_t driver_read(struct file *File, char *user_buffer, size_t count, loff_t *offs){
	int to_copy, not_copied, delta;
	char out_string[40];
	int temperature;
	int pressure;

	/* Get amount of bytes to copy */
	to_copy = min(sizeof(out_string), count);

	/* Get temperature */
	temperature = read_temperature();
	/* Get pressure */
	pressure = read_pressure();
	snprintf(out_string, sizeof(out_string), "%d.%d\n%d.%d\n", temperature/100, temperature%100,pressure/100, pressure%100);

	/* Copy Data to user */
	not_copied = copy_to_user(user_buffer, out_string, to_copy);

	/* Calculate delta */
	delta = to_copy - not_copied;

	return delta;
}


/**
 * @brief Write data to buffer
 */
static ssize_t driver_write(struct file *File,const char *user_buffer, size_t count, loff_t *offs){

	printk("BME280 - Cannot write to read-only sensor\n");
	return -1;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int driver_open(struct inode *device_file, struct file *instance){
	printk("BME280 - open was called!\n");
	return 0;
}

/**
 * @brief This function is called, when the device file is closed
 */
static int driver_close(struct inode *device_file, struct file *instance){
	printk("BME280 - close was called!\n");
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
	int ret = -1;
	u8 id;

	printk("BME280 - Hello, Kernel!\n");

	/* Allocate a device number */
	if (alloc_chrdev_region(&my_device_number, 0 ,1, DRIVER_NAME) < 0){
		printk("BME280 - Device number could not be allocated!\n"); 
		return -1;
	}

	printk("BME280 - Device number Major: %d, Minor %d was registered!\n", my_device_number >> 20, my_device_number & 0xfffff);
	        	
	/* Create device class */
	if ((my_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL){
		printk("BME280 - Device class can not be created!\n");
		goto ClassError;
	}

	/* Create a device file  */
	if(device_create(my_class, NULL, my_device_number, NULL, DRIVER_NAME) == NULL){
		printk("BME280 - Device file can not be created!\n");
		goto FileError;
	}

	/* Initialize device file */
	cdev_init(&my_device, &fops);

	/* Registering device to kernel */
	if(cdev_add(&my_device, my_device_number, 1) < 0 ){
		printk("BME280 - Registering of device to kernel failed!\n");
		goto KernelError;
	}

	/* Create adapter to get access on the BUS */
	bme280_i2c_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);

	if(bme280_i2c_adapter != NULL){
		bme280_i2c_client = i2c_new_client_device(bme280_i2c_adapter, &bme280_i2c_board_info);
		if(bme280_i2c_client != NULL){
			if(i2c_add_driver(&bme280_driver) != -1){
				ret = 0;
			}else{
				printk("BME280 - Can't add driver\n");
			}
		}
		i2c_put_adapter(bme280_i2c_adapter);
	}

	printk("BME280 - Driver added!\n");	  
	
	/* Read ID */
	id = i2c_smbus_read_byte_data(bme280_i2c_client, 0xD0); 
	printk("BME280 - ID 0x%x\n", id);

	/* Read Calibration Values */
	dig_T1 = i2c_smbus_read_word_data(bme280_i2c_client,0x88); 
	dig_T2 = i2c_smbus_read_word_data(bme280_i2c_client,0x8A); 
	dig_T3 = i2c_smbus_read_word_data(bme280_i2c_client,0x8C); 
	dig_P1 = i2c_smbus_read_word_data(bme280_i2c_client,0x8E);
	dig_P2 = i2c_smbus_read_word_data(bme280_i2c_client,0x90);
	dig_P3 = i2c_smbus_read_word_data(bme280_i2c_client,0x92);
	dig_P4 = i2c_smbus_read_word_data(bme280_i2c_client,0x94);
	dig_P5 = i2c_smbus_read_word_data(bme280_i2c_client,0x96);
	dig_P6 = i2c_smbus_read_word_data(bme280_i2c_client,0x98);
	dig_P7 = i2c_smbus_read_word_data(bme280_i2c_client,0x9A);
	dig_P8 = i2c_smbus_read_word_data(bme280_i2c_client,0x9C);
	dig_P9 = i2c_smbus_read_word_data(bme280_i2c_client,0x9E);

	/* Initialize device */
	/* sample data every second */
	i2c_smbus_write_byte_data(bme280_i2c_client, 0xf5, 0x5 << 7);

	/* oversampling * 16 for temperature and pressure */
	i2c_smbus_write_byte_data(bme280_i2c_client, 0xf4,(5 <<5) | (5<<2) | (3 <<0 ));
	 
	return ret;
KernelError:
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
	i2c_unregister_device(bme280_i2c_client);
	i2c_del_driver(&bme280_driver);
	cdev_del(&my_device);
	device_destroy(my_class, my_device_number);
	class_destroy(my_class);
	unregister_chrdev(my_device_number, DRIVER_NAME);

	printk("BME280 - Goodbye, Kernel!\n");

}

module_init(ModuleInit);
module_exit(ModuleExit);

