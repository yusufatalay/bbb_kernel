#include <linux/module.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/string.h>


/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yusuf Taha Atalay");
MODULE_DESCRIPTION("An example LKM that creates sys entry");

/*
 * @brief Read callback for hello/dummy
 */
static ssize_t dummy_show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer){
	return sprintf(buffer, "You have read from /sys/kernel/%s/%s\n",kobj->name, attr->attr.name);
}

/*
 * @brief Write callback for hello/dummy
 */
static ssize_t dummy_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count){
	printk("sysfs_test: You wrote '%s' to /sys/kernel/%s/%s\n", buffer, kobj->name, attr->attr.name);
	return count;

}


/* Global variable for sysfs folder hello */
static struct kobject *dummy_kobject;

static struct kobj_attribute dummy_attribute = __ATTR(dummy, 0660,dummy_show ,dummy_store );

/*
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init ModuleInit(void){
	printk("sysfs_test: creating /sys/kernel/hello/dummy!\n");

	/* Creating the folder hello */
	dummy_kobject = kobject_create_and_add("hello", kernel_kobj);
	if (dummy_kobject == NULL){
		printk("sysfs_test: failed to create folder  /sys/kernel/hello/!\n");
		return -ENOMEM;
	}

	/* Creating the sysfs file dummy */
	if (sysfs_create_file(dummy_kobject, &dummy_attribute.attr)){
		kobject_put(dummy_kobject);
		printk("sysfs_test: failed to create   /sys/kernel/hello/dummy\n");
		return -ENOMEM;
	}


	return 0;
}

/*
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit ModuleExit(void){
	sysfs_remove_file(dummy_kobject, &dummy_attribute.attr);
	kobject_put(dummy_kobject);	// this one also removes the file 
	printk("sysfs_test: De-Initialized!\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);

