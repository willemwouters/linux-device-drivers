#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

// module attributes
MODULE_LICENSE("GPL"); // this avoids kernel taint warning
MODULE_DESCRIPTION("Device Driver");
MODULE_AUTHOR("WILLEM WOUTERS");

static char msg[100]={0};
static short readPos=0;
static int times = 0;
static const char * dname = "myDev";

static dev_t first; // Global variable for the first device number
static struct cdev c_dev; // Global variable for the character device structure
static struct class *cl; // Global variable for the device class

// protoypes,else the structure initialization that follows fail
static int dev_open(struct inode *, struct file *);
static int dev_rls(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

// structure containing callbacks
static struct file_operations fops = 
{
    .owner = THIS_MODULE,
	.read = dev_read, // address of dev_read
	.open = dev_open,  // address of dev_open
	.write = dev_write, // address of dev_write 
	.release = dev_rls, // address of dev_rls
};

// called when module is loaded, similar to main()
int init_module(void)
{
      if (alloc_chrdev_region(&first, 0, 1, dname) < 0)
      {
        return -1;
      }
      if ((cl = class_create(THIS_MODULE, dname)) == NULL)
      {
         unregister_chrdev_region(first, 1);
         return -1;
     }
     if (device_create(cl, NULL, first, NULL,  dname) == NULL)
     {
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return -1;
     }
     cdev_init(&c_dev, &fops);
     if (cdev_add(&c_dev, first, 1) == -1)
     {
       device_destroy(cl, first);
       class_destroy(cl);
       unregister_chrdev_region(first, 1);
       return -1;
     }
    printk(KERN_INFO "Device registered...by Willem Wouters \n");
	return 0;
}


// called when module is unloaded, similar to destructor in OOP
void cleanup_module(void)
{
     cdev_del(&c_dev);
     device_destroy(cl, first);
     class_destroy(cl);
     unregister_chrdev_region(first, 1);
     printk(KERN_INFO "Unregistered device....");
}


// called when 'open' system call is done on the device file
static int dev_open(struct inode *inod,struct file *fil)
{	
	times++;
	printk(KERN_ALERT"Device opened %d times\n",times);
	return 0;
}

// called when 'read' system call is done on the device file
static ssize_t dev_read(struct file *filp,char *buff,size_t len,loff_t *off)
{
	short count = 0;
	while (len && (msg[readPos]!=0))
	{
		put_user(msg[readPos],buff++); //copy byte from kernel space to user space
		count++;
		len--;
		readPos++;
	}
	return count;
}

// called when 'write' system call is done on the device file
static ssize_t dev_write(struct file *filp,const char *buff,size_t len,loff_t *off)
{
	short ind = len-1;
	short count=0;
	memset(msg,0,100);
	readPos=0;
	while(len>0)
	{
		msg[count++] = buff[ind--]; //copy the given string to the driver but in reverse
		len--;
	}
    printk(KERN_INFO "Wrote to device: %s \n", msg);
	return count;
}

// called when 'close' system call is done on the device file
static int dev_rls(struct inode *inod,struct file *fil)
{
	printk(KERN_ALERT"Device closed\n");
	return 0;
}
