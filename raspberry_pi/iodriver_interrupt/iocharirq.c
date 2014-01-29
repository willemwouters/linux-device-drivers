#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/semaphore.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/sched.h>

#include <linux/init.h> 
#include <linux/interrupt.h>
#include <linux/gpio.h>


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
    wake_up_interruptible(&wq);
    printk(KERN_INFO "Interrupt [%d] %d %s for device %s was triggered, val: %d!.\n",   irq, irqmaps[gpio_num].irq, irqmaps[gpio_num].gpiostruct.label, (char *) dev_id, val);
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
    return 0;
}

void driver_cleanup(void) {
    printk(KERN_INFO "Simple-driver: unloading driver\n");
    unregister_chrdev_region(device_file_major_number, 1);
    cdev_del(kernel_cdev);
    free_irq_gpio(irqmaps, sizeof(irqmaps) / structsize);
}

module_init(driver_init);
module_exit(driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
