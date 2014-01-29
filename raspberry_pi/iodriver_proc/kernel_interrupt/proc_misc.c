

static int proc_write(struct file *file, const char __user *buffer, unsigned long count, void *data)
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
	pid = (int) myLong;

	if(pid != 0) 
	printk("write_first received data: %d\n", pid);
	return count; 
}


static struct file_operations proc_fops = {
	.write = proc_write,
};
