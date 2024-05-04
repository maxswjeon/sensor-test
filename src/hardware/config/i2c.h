#ifndef __HARDWARE_CONF_I2C_H__
#define __HARDWARE_CONF_I2C_H__

#include <stddef.h>

#include <hardware/i2c.h>

typedef struct
{
	uint8_t sda;
	uint8_t scl;
	uint32_t freq;

	i2c_inst_t *inst;
} i2c_conf_t;

int setup_i2c(i2c_conf_t *);
int validate_i2c(i2c_conf_t *);

#endif // __HARDWARE_CONF_I2C_H__