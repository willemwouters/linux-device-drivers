#include <asm/siginfo.h>	//siginfo
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>

#include "gpio_misc.h"

extern irqreturn_t r_irq_handler(int irq, void *dev_id, struct pt_regs *regs);

int gpio_request_arr(struct Irqmap leds_gpiostruct[], int size) {
    int i;
    for(i = 0; i < size; i++) {
       gpio_request(leds_gpiostruct[i].gpiostruct.gpio, leds_gpiostruct[i].gpiostruct.label);
    }
    return 0;
}


int request_irq_array(struct Irqmap leds_gpiostruct[], int size) {
    int i;
    for(i = 0; i < size; i++) {
        if ( (leds_gpiostruct[i].irq = gpio_to_irq(leds_gpiostruct[i].gpiostruct.gpio)) < 0 ) {
            printk("GPIO to IRQ mapping faiure %s\n", leds_gpiostruct[i].gpiostruct.label);
            return 1;
        }

        if (request_irq(leds_gpiostruct[i].irq, (irq_handler_t ) r_irq_handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, leds_gpiostruct[i].gpiostruct.label, leds_gpiostruct[i].gpiostruct.label)) {
            printk("Irq Request failure\n");
            return 1;
        }
        printk("GPIO %d %s to IRQ mapping %d\n", leds_gpiostruct[i].gpiostruct.gpio, leds_gpiostruct[i].gpiostruct.label, leds_gpiostruct[i].irq);
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
