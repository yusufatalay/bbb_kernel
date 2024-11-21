#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/string.h>


/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yusuf Taha Atalay");
MODULE_DESCRIPTION("Demonstration for dynamic memory management in a LKM");

struct driver_text{
	u8 version;
	char text[64];
};

u32 *ptr1;
struct driver_text *ptr2;

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init ModuleInit(void){
	printk("alloc_test: Initiating ptr1 with kmalloc\n");
	ptr1 = kmalloc(sizeof(u32), GFP_KERNEL);
	if(ptr1 == NULL){
		printk("alloc_test: Out of memory\n");
		return -ENOMEM;
	}
	printk("alloc_test: *ptr1: 0x%x\n", *ptr1);
	*ptr1 = 0xdeadbeef;
	printk("alloc_test: *ptr1: 0x%x\n", *ptr1);
	kfree(ptr1);

	printk("alloc_test: Initiating ptr1 with kzalloc\n");
	ptr1 = kzalloc(sizeof(u32), GFP_KERNEL);
	if(ptr1 == NULL){
		printk("alloc_test: Out of memory\n");
		return -ENOMEM;
	}
	printk("alloc_test: *ptr1: 0x%x\n", *ptr1);
	*ptr1 = 0xdeadbeef;
	printk("alloc_test: *ptr1: 0x%x\n", *ptr1);
	kfree(ptr1);

	ptr2 = kzalloc(sizeof(struct driver_text), GFP_KERNEL);
	if (ptr2 == NULL){
		printk("alloc_test: Out of memory\n");
		return -ENOMEM;
	}

	ptr2->version = 123;
	strcpy(ptr2->text, "TEST DRIVER STRING");

	printk("alloc_tests: Driver Version %d, Driver Data %s\n", ptr2->version, ptr2->text);	

	return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit ModuleExit(void){
	printk("alloc_tests: Driver Version %d, Driver Data %s\n", ptr2->version, ptr2->text);	

	kfree(ptr2);
}

module_init(ModuleInit);
module_exit(ModuleExit);

