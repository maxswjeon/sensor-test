#ifndef __HARDWARE_CONF_SPI_H__
#define __HARDWARE_CONF_SPI_H__

#include <stddef.h>

#include <hardware/spi.h>

typedef struct
{
	uint8_t miso;
	uint8_t mosi;
	uint8_t sck;
	uint8_t cs;

	uint32_t freq;

	spi_inst_t *inst;
} spi_conf_t;

int setup_spi(spi_conf_t *);
int validate_spi(spi_conf_t *);
int hardware_cs(spi_conf_t *);

#endif // __HARDWARE_CONF_I2C_H__