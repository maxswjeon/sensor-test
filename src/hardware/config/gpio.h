#ifndef __HARDWARE_CONF_GPIO_H__
#define __HARDWARE_CONF_GPIO_H__

#include <stddef.h>

typedef struct
{
	uint8_t pin;
	uint8_t dir;
} gpio_conf_t;

#endif // __HARDWARE_CONF_GPIO_H__