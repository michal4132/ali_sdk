#ifndef _GPIO_H_
#define _GPIO_H_

#include <stdint.h>

#define HAL_GPIO_I_DIR		0
#define HAL_GPIO_O_DIR		1

#define GPIO_GROUP_MAX	    5
#define GPIO_PORT_MAX		159

#define __REGALIRAW(x)        (x)

typedef unsigned int DWORD;
int gpio_is_valid(int gpio);
int gpio_request(unsigned gpio, const char *label);
int gpio_free(unsigned gpio);
int gpio_direction_input(unsigned gpio);
int gpio_direction_output(unsigned gpio, int value);
int gpio_get_value(unsigned gpio);
int gpio_set_value(unsigned gpio, int value);
int gpio_get_interrupt_status(unsigned gpio);
int gpio_clear_interrupt_status(unsigned gpio);
int gpio_interrupt_enable(unsigned gpio,int rise_en, int fall_en);
int gpio_interrupt_disable(unsigned gpio);

#endif // _GPIO_H_