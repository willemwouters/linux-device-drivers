#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/semaphore.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/init.h> 
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <asm/siginfo.h>	//siginfo
#include <linux/rcupdate.h>	//rcu_read_lock
#include <linux/sched.h>	//find_task_by_pid_type
#include <linux/uaccess.h>


typedef struct Irqmap {
	int irq;
    struct gpio gpiostruct;
} Irqmap;

int structsize = 16;
 Irqmap irqmaps[] = {
		{ 0, { 17, GPIOF_OUT_INIT_HIGH, "Power LED" } },
		{ 0, { 18, GPIOF_OUT_INIT_HIGH, "Green LED" } },
		{ 0, { 19, GPIOF_OUT_INIT_HIGH, "Red LED"   } },
		{ 0, { 20, GPIOF_OUT_INIT_HIGH, "Blue LED"  } }
	};


//////////// PROC DEV  //////////
#define SIG_TEST 44	// we choose 44 as our signal number (real-time signals are in the range of 33 to 64)
struct proc_dir_entry *proc_entry;
const char * proc_name = "iodriverpid";
int pid = 0;

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
#define DRIVER_AUTHOR "Igor <hardware.coder@gmail.com>"
#define DRIVER_DESC   "Tnterrupt Test"


int open(struct inode *inode, struct file *filp) {
   printk(KERN_INFO "open:\n"); 
   return 0;
}

int release(struct inode *inode, struct file *filp) {
    printk(KERN_INFO "release:\n");
    return 0;
}




static ssize_t read(struct file *file_ptr, char __user *user_buffer, size_t count, loff_t *position)
{
    printk( KERN_NOTICE "Simple-driver: Device file is read at offset = %i, read bytes count = %u" , (int)*position , (unsigned int)count );

    wait_event_interruptible(wq, flag != 0); 
    flag = 0;   /* what happens if this is set to 0? */

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


static int send_signal(int myPid) {
	/* send the signal */
	struct siginfo info;
	int ret;
	struct task_struct *t;
	struct pid *pid_struct;
	pid_t pid;
	memset(&info, 0, sizeof(struct siginfo));
	info.si_signo = SIG_TEST;
	info.si_code = SI_QUEUE;	// this is bit of a trickery: SI_QUEUE is normally used by sigqueue from user space,
					// and kernel space should use SI_KERNEL. But if SI_KERNEL is used the real_time data 
					// is not delivered to the user space signal handler function. 
	info.si_int = 1;  		//real time signals may have 32 bits of data.

	rcu_read_lock();

	pid = myPid; //integer value of pid
	pid_struct = find_get_pid(pid); //function to find the pid_struct
	t = pid_task(pid_struct,PIDTYPE_PID); //find the task_struct
	if(t == NULL){
		printk("no such pid\n");
		rcu_read_unlock();
		return -ENODEV;
	}
	rcu_read_unlock();
	ret = send_sig_info(SIG_TEST, &info, t);    //send the signal
	if (ret < 0) {
		printk("error sending signal\n");
		return ret;
	}
	return 0;
}

static irqreturn_t r_irq_handler(int irq, void *dev_id, struct pt_regs *regs) {
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
    
    printk(KERN_INFO "Interrupt PID=%d [%d] %d %s for device %s was triggered, val: %d!.\n", pid,  irq, irqmaps[gpio_num].irq, irqmaps[gpio_num].gpiostruct.label, (char *) dev_id, val);
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


int request_irq_array(struct Irqmap leds_gpiostruct[], int size) {
    int i;
    for(i = 0; i < size; i++) {
        if ( (irqmaps[i].irq = gpio_to_irq(leds_gpiostruct[i].gpiostruct.gpio)) < 0 ) {
            printk("GPIO to IRQ mapping faiure %s\n", leds_gpiostruct[i].gpiostruct.label);
            return 1;
        }

        if (request_irq(irqmaps[i].irq, (irq_handler_t ) r_irq_handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, leds_gpiostruct[i].gpiostruct.label, leds_gpiostruct[i].gpiostruct.label)) {
            printk("Irq Request failure\n");
            return 1;
        }
        printk("GPIO %d %s to IRQ mapping %d\n", leds_gpiostruct[i].gpiostruct.gpio, leds_gpiostruct[i].gpiostruct.label, leds_gpiostruct[i].irq);
    }
	return 0;
}

int gpio_request_arr(struct Irqmap leds_gpiostruct[], int size) {
    int i;
    for(i = 0; i < size; i++) {
       gpio_request(leds_gpiostruct[i].gpiostruct.gpio, leds_gpiostruct[i].gpiostruct.label);
    }
    return 0;
}

int free_irq_gpio(struct Irqmap leds_gpiostruct[], int size) {
    int i;
    for(i = 0; i < size; i++) {
        free_irq(leds_gpiostruct[i].irq, leds_gpiostruct[i].gpiostruct.label);
        gpio_free(leds_gpiostruct[i].gpiostruct.gpio);
    }
    return 0;
}


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

int driver_init (void) {
    int ret;
    int err;
   
    ///////////////// CHAR DEV ///////////////////////
    kernel_cdev = cdev_alloc();    
    kernel_cdev->ops = &fops; 
    kernel_cdev->owner = THIS_MODULE;
    printk(KERN_INFO "Simple-driver: starting driver \n");
    ret = register_chrdev( 0, device_name, &fops );
    if( ret < 0 )
    {
        printk( KERN_INFO "Simple-driver:  can\'t register character device with errorcode = %i", ret );
        return ret;
    }
    device_file_major_number = ret;
    printk( KERN_INFO "Simple-driver: registered character device with major number = %i and minor numbers 0...255 \r\n" , device_file_major_number );

    ///////////////// IRQ //////////////////////////////
    err = gpio_request_arr(irqmaps, sizeof(irqmaps) / structsize);
    if (err) {
        return -1;
    }

    err = request_irq_array(irqmaps, sizeof(irqmaps) / structsize);
    if(err) {
        return -1;
    }

    ///////////////// PROC //////////////////////////
    proc_entry = proc_create(proc_name, 0444, NULL, &proc_fops);
    if (!proc_entry)
        return -EPERM;

    return 0;
}

void driver_cleanup(void) {
    printk(KERN_INFO "Simple-driver: unloading driver\n");
    unregister_chrdev_region(device_file_major_number, 1);
    cdev_del(kernel_cdev);
    free_irq_gpio(irqmaps, sizeof(irqmaps) / structsize);
    remove_proc_entry(proc_name, NULL);
}

module_init(driver_init);
module_exit(driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
