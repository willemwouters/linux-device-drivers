


static int proc_write(struct file *file, const char __user *buffer, unsigned long count, void *data);

static struct file_operations proc_fops = {
	.write = proc_write,
};
