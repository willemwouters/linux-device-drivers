#include "proc_misc.h"
#include <linux/proc_fs.h>
#include <linux/uaccess.h>


extern int *proc_pid;


struct file_operations proc_fops = {
	.write = procm_write,
};


struct proc_dir_entry* procm_create(const char * proc_name) {
   struct  proc_dir_entry *proc_entry = proc_create(proc_name, 0444, NULL, &proc_fops);
   return proc_entry;
}

void procm_delete(const char * proc_name) {
    remove_proc_entry(proc_name, NULL);
}


int procm_write(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	long myLong;
	char buff[10];
	if (count < 0 || count > 10)
		return -EFAULT;

        if(copy_from_user(buff, buffer, count)) {
		return -EFAULT;
        }
	buff[count] = '\0';

    if (kstrtol(buff, 10, &myLong) == 0)
    {
		printk("We have a number %d!\n", (int) myLong);
    }
	*proc_pid = (int) myLong;

	if(*proc_pid != 0) 
	printk("write_first received data: %d\n", *proc_pid);
	return count; 
}

