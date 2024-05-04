#ifndef __HARDWARE_BHY2_H__
#define __HARDWARE_BHY2_H__

#include <hardware/i2c.h>
#include <hardware/spi.h>
#include <hardware/gpio.h>

#include <hardware/config/i2c.h>
#include <hardware/config/spi.h>
#include <hardware/config/gpio.h>

typedef struct
{
	i2c_inst_t *i2c;
	spi_inst_t *spi;
	gpio_conf_t *intr;
	gpio_conf_t *rset;
} bhy2_inst_t;

typedef struct
{
	i2c_conf_t *i2c;
	spi_conf_t *spi;
	gpio_conf_t *intr;
	gpio_conf_t *rset;
} bhy2_conf_t;

#endif // __HARDWARE_BHY2_H__