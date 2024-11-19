#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/i2c.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yusuf Taha Atalay");
MODULE_DESCRIPTION("An example I2C Driver for BME280 LKM");

static struct i2c_client *bme_client;

/* Declare the probe and remove functions */
static int my_bme_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int my_bme_remove(struct i2c_client *client);

static struct of_device_id my_driver_ids[] = {
	{
		.compatible = "brightlight,mybme",
	}, { /* sentinel */}

};

MODULE_DEVICE_TABLE(of, my_driver_ids);

static struct i2c_device_id my_bme[]= {
	{"my_bme", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, my_bme );

static struct i2c_driver my_driver = {
	.probe = my_bme_probe,
	.remove = my_bme_remove,
	.id_table = my_bme,
	.driver = {
		.name = "my_bme",
		.of_match_table = my_driver_ids,
	},
};

/* Global variables for procfs file */
static struct proc_dir_entry *proc_file;

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
	d1 = i2c_smbus_read_byte_data(bme_client, 0xFA);
	d2 = i2c_smbus_read_byte_data(bme_client, 0xFB);
	d3 = i2c_smbus_read_byte_data(bme_client, 0xFC);
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
	d1 = i2c_smbus_read_byte_data(bme_client, 0xF7);
	d2 = i2c_smbus_read_byte_data(bme_client, 0xF8);
	d3 = i2c_smbus_read_byte_data(bme_client, 0xF9);
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


/*
 * @brief Write data to buffer
 * For future: support sensor configuration in here
 */
static ssize_t my_write(struct file *File, const char __user *user_buffer, size_t count, loff_t *offs) {
    return count; // Simply consume the data without any action
}

/*
 * @brief Read temprature and pressure value
 */
static ssize_t my_read(struct file *File, char __user *user_buffer, size_t count, loff_t *offs) {
    char out_string[128]; // Kernel buffer to format the output
    int len;              // Length of the formatted string
    int temperature;
    int pressure;

    printk("dt_i2c: in my_read function\n");

    /* Prevent multiple reads from returning duplicate data */
    if (*offs > 0){
        return 0;
    }

    /* Get temperature and pressure values */
    temperature = read_temperature();
    pressure = read_pressure();

    /* Format the output string */
    len = snprintf(out_string, sizeof(out_string),
                   "Temp: %d.%d Celcius\nPressure: %d.%d Pascal\n",
                   temperature / 100, temperature % 100,
                   pressure / 100, pressure % 100);

    /* Ensure we don't copy more than the buffer size in user space */
    if (len > count){
        len = count;
    }

    /* Copy data to user space */
    if (copy_to_user(user_buffer, out_string, len)) {
        printk("dt_i2c: Failed to copy data to user space\n");
        return -EFAULT; // Return error if copy fails
    }

    /* Update file offset to prevent multiple reads of the same data */
    *offs = len;

    return len; // Return number of bytes copied to user buffer
}


static struct proc_ops fops = {
	.proc_write = my_write,
	.proc_read = my_read,
};


/*
 * @brief This function is called on loading the driver
 */
static int my_bme_probe(struct i2c_client *client, const struct i2c_device_id *dev_id){
	printk("dt_i2c: in the probe function\n");

	if (client-> addr != 0x76){
		printk("dt_i2c: Could not found a BME280 on i2c bus 2\n");
		return -1;
	}

	bme_client = client;

	/* Read ID */
	s32 id = i2c_smbus_read_byte_data(bme_client, 0xD0); 
	printk("BME280 - ID 0x%x\n", id);

	/* Read Calibration Values */
	dig_T1 = i2c_smbus_read_byte_data(bme_client,0x88); 
	dig_T2 = i2c_smbus_read_byte_data(bme_client,0x8A); 
	dig_T3 = i2c_smbus_read_byte_data(bme_client,0x8C); 
	dig_P1 = i2c_smbus_read_byte_data(bme_client,0x8E);
	dig_P2 = i2c_smbus_read_byte_data(bme_client,0x90);
	dig_P3 = i2c_smbus_read_byte_data(bme_client,0x92);
	dig_P4 = i2c_smbus_read_byte_data(bme_client,0x94);
	dig_P5 = i2c_smbus_read_byte_data(bme_client,0x96);
	dig_P6 = i2c_smbus_read_byte_data(bme_client,0x98);
	dig_P7 = i2c_smbus_read_byte_data(bme_client,0x9A);
	dig_P8 = i2c_smbus_read_byte_data(bme_client,0x9C);
	dig_P9 = i2c_smbus_read_byte_data(bme_client,0x9E);

	/* Initialize device */
	/* sample data every second */
	i2c_smbus_write_byte_data(bme_client, 0xf5, 0x5 << 7);

	/* oversampling * 16 for temperature and pressure */
	i2c_smbus_write_byte_data(bme_client, 0xf4,(5 <<5) | (5<<2) | (3 <<0 ));


	/* Creating procfs file */
	proc_file = proc_create("mybme", 0666,NULL , &fops);
	if(proc_file == NULL){
		printk("dt_i2c: FAILED TO CREATE FILE  /proc/mybme\n");
		return -ENOMEM;
	}	
	
	printk("dt_i2c: probing successful\n");

	return 0;
}

/*
 * @brief This function is called on unloading the driver
 */
static int my_bme_remove(struct i2c_client *client){
	printk("dt_i2c: in the remove function\n");
	proc_remove(proc_file);

	return 0;
}

/* This will create the init and exit function automatically */
module_i2c_driver(my_driver);
