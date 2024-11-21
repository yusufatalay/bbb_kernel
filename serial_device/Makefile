obj-m += serial_device.o

all : module dt

module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

dt: serial_device_overlay.dts
	dtc -@ -I dts -O dtb -o serial_device_overlay.dtbo serial_device_overlay.dts
	
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf serial_device_overlay.dtbo

	
