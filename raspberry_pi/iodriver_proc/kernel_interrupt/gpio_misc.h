#ifndef _GPIO_MISC_H
#define _GPIO_MISC_H

#include <linux/gpio.h>



typedef struct Irqmap {
	int irq;
    struct gpio gpiostruct;
} Irqmap;



int free_irq_gpio(struct Irqmap leds_gpiostruct[], int size);

int gpio_request_arr(struct Irqmap leds_gpiostruct[], int size);

int request_irq_array(struct Irqmap leds_gpiostruct[], int size);

#endif //_GPIO_MISC_H
