

typedef struct Irqmap {
	int irq;
    struct gpio gpiostruct;
} Irqmap;



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
