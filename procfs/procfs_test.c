#include <linux/module.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/proc_fs.h>


/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yusuf Taha Atalay");
MODULE_DESCRIPTION("An example LKM that creates procfs entry");

/* Global variables for procfs folder and file */
static struct proc_dir_entry *proc_folder;
static struct proc_dir_entry *proc_file;

/*
 * @brief Read data out of the buffer
 */
static ssize_t my_read(struct file *File, char *user_buffer, size_t count, loff_t *offs){
	char text[] = "Hello from a procfs file!\n";
	int to_copy, not_copied, delta;

	/* Get amount of data to copy */
	to_copy = min(count, sizeof(text));

	/* Copy data to user */
	not_copied = copy_to_user(user_buffer, text, to_copy);
	printk("procfs_test - You have written %s to me\n", text);

	/* Calculate data */
	delta = to_copy - not_copied;

	return delta;

}

/*
 * @brief Write data to buffer
 */
static ssize_t my_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs){
	char text[255];
	int to_copy, not_copied, delta;

	/* Clear text */
	memset(text, 0 ,sizeof(text));

	/* Get amount of data to copy */
	to_copy = min(count, sizeof(text));

	/* Copy data to user */
	not_copied = copy_from_user(text, user_buffer, to_copy);
	printk("procfs_test - You have written %s to me\n", text);

	/* Calculate data */
	delta = to_copy - not_copied;

	return delta;
}


static struct proc_ops fops = {
	.proc_read = my_read,
	.proc_write = my_write,
};

/*
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init ModuleInit(void){
	printk("procfs_test - initialized!\n");

	/* /proc/hello/dummy */
	proc_folder = proc_mkdir("hello", NULL);
	if(proc_folder == NULL){
		printk("procfs_test - FAILED TO CREATE FOLDER under /proc\n");
		return -ENOMEM;
	}

	proc_file = proc_create("dummy", 0666, proc_folder, &fops);
	if(proc_file == NULL){
		printk("procfs_test - FAILED TO CREATE FILE  under /proc/dummy\n");
		proc_remove(proc_folder);
		return -ENOMEM;
	}	
	
	printk("procfs_test - Created /proc/hello/dummy\n");

	return 0;
}

/*
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit ModuleExit(void){
	printk("procfs_test - removing /proc/hello/dummy!\n");
	proc_remove(proc_file);
	proc_remove(proc_folder);

}

module_init(ModuleInit);
module_exit(ModuleExit);

