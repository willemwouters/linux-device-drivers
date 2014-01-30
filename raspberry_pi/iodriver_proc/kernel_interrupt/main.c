#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/semaphore.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/init.h> 
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <asm/siginfo.h>	//siginfo
#include <linux/rcupdate.h>	//rcu_read_lock
#include <linux/sched.h>	//find_task_by_pid_type

#include "gpio_misc.h"
#include "proc_misc.h"
#include "signal_misc.h"



int structsize = 16;
 Irqmap irqmaps[] = {
		{ 0, { 17, GPIOF_OUT_INIT_HIGH, "Power LED" } },
		{ 0, { 18, GPIOF_OUT_INIT_HIGH, "Green LED" } },
		{ 0, { 19, GPIOF_OUT_INIT_HIGH, "Red LED"   } },
		{ 0, { 20, GPIOF_OUT_INIT_HIGH, "Blue LED"  } }
	};


//////////// PROC DEV  //////////
struct proc_dir_entry *proc_entry;
const char * proc_name = "iodriverpid";
int pid = 0;
int *proc_pid;

//////////// CHAR DEV ///////////
static int device_file_major_number = 0;
static const char device_name[] = "GPIO-Interrupt-driver";
struct cdev *kernel_cdev; 
static int flag = 0;
static DECLARE_WAIT_QUEUE_HEAD(wq);

static char return_string[100] = { '\0' };
static int return_size = sizeof(return_string);

//////////// IRQ /////////////////
short int irq_any_gpio    = 0;
#define DRIVER_AUTHOR "Willem Wouters <willemwouters@gmail.com>"
#define DRIVER_DESC   "GPIO Interrupt driver"

int open(struct inode *inode, struct file *filp) {
   printk(KERN_INFO "%s: open:\n", device_name); 
   return 0;
}

int release(struct inode *inode, struct file *filp) {
    printk(KERN_INFO "%s: release:\n", device_name);
    return 0;
}


static ssize_t read(struct file *file_ptr, char __user *user_buffer, size_t count, loff_t *position)
{
    printk( KERN_NOTICE "%s: Device file is read at offset = %i, read bytes count = %u" , device_name, (int)*position , (unsigned int)count );

    wait_event_interruptible(wq, flag != 0); 
    flag = 0;  

    /* If position is behind the end of a file we have nothing to read, start all over */
    if( *position >= return_size ) {
        *position -= return_size;
    }

    /* If a user tries to read more than we have, read only 	as many bytes as we have */
    if( *position + count > return_size)
        count = return_size - *position;

    if( copy_to_user(user_buffer, return_string + *position, count) != 0 )
        return -EFAULT;	

    /* Move reading position */
    *position += count;
    return count;
}


ssize_t write(struct file *filp, const char *b, size_t len, loff_t *o) {
    short ind = len-1;
    short count = 0;
    memset(return_string, 0, 100);
    while(len > 0) 
    {
        return_string[count++] = b[ind--];
        len--;
    }

    flag = 1;
    wake_up_interruptible(&wq);
    return count;
}



irqreturn_t r_irq_handler(int irq, void *dev_id, struct pt_regs *regs) {
    unsigned long flags;
    int i;
    int gpio_num = -1;
    int val;
    local_irq_save(flags);
    flag = 1;
    for(i = 0; i < sizeof(irqmaps) / structsize; i++) {
        if(irqmaps[i].irq == irq) {
            gpio_num = i;
        }
    }
    val = gpio_get_value(irqmaps[gpio_num].gpiostruct.gpio);
    return_string[0] = (irqmaps[gpio_num].gpiostruct.gpio / 10) + 48;
    return_string[1] = (irqmaps[gpio_num].gpiostruct.gpio % 10) + 48;;
    return_string[2] = ':';
    return_string[3] = val + 48;
    return_string[4] = '\n';
    return_string[5] = '\0';
    return_size = 6;
    if(pid > 0) {
	send_signal(pid);
    }
    wake_up_interruptible(&wq);
    
    printk(KERN_INFO "%s: Interrupt PID=%d [%d] %d %s for device %s was triggered, val: %d!.\n", device_name, pid,  irq, irqmaps[gpio_num].irq, irqmaps[gpio_num].gpiostruct.label, (char *) dev_id, val);
    // restore hard interrupts
    local_irq_restore(flags);
    return IRQ_HANDLED;
}


struct file_operations fops = { 
   .owner = THIS_MODULE,
   .read = read, 
   .write = write, 
   .open = open, 
   .release = release
};


int driver_init (void) {
    int ret;
    int err;
   
    ///////////////// CHAR DEV ///////////////////////
    kernel_cdev = cdev_alloc();    
    kernel_cdev->ops = &fops; 
    kernel_cdev->owner = THIS_MODULE;
    printk(KERN_INFO "%s: starting driver \n", device_name);
    ret = register_chrdev( 0, device_name, &fops );
    if( ret < 0 )
    {
        printk( KERN_INFO "%s:  can\'t register character device with errorcode = %i", device_name, ret );
        return ret;
    }
    device_file_major_number = ret;
    printk( KERN_INFO "%s: registered character device with major number = %i and minor numbers 0...255 \r\n" , device_name, device_file_major_number );

    ///////////////// IRQ //////////////////////////////
    err = gpio_request_arr(irqmaps, sizeof(irqmaps) / structsize);
    if (err) {
        return -1;
    }
    err = request_irq_array(irqmaps, sizeof(irqmaps) / structsize);
    if(err) {
        return -1;
    }

    ////////////////////// PROC ////////////////////
    proc_pid = &pid;
    proc_entry = procm_create(proc_name);
    return 0;
}

void driver_cleanup(void) {
    printk(KERN_INFO "%s: unloading driver\n", device_name);
    unregister_chrdev_region(device_file_major_number, 1);
    cdev_del(kernel_cdev);
    free_irq_gpio(irqmaps, sizeof(irqmaps) / structsize);
    procm_delete(proc_name);
}

module_init(driver_init);
module_exit(driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
