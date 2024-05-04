#ifdef PICO_BUILD

#include <stdbool.h>

#include <pico/stdlib.h>
#include <hardware/config/spi.h>

int setup_spi(spi_conf_t *conf)
{
	int valid = validate_spi(conf);
	if (valid != 0)
	{
		return valid;
	}

	spi_init(conf->inst, conf->freq);
	gpio_set_function(conf->miso, GPIO_FUNC_SPI);
	gpio_set_function(conf->mosi, GPIO_FUNC_SPI);
	gpio_set_function(conf->sck, GPIO_FUNC_SPI);

	if (hardware_cs(conf))
	{
		gpio_set_function(conf->cs, GPIO_FUNC_SPI);
	}

	return 0;
}

uint8_t validate_spi(spi_conf_t *conf)
{
	if (conf->inst == NULL)
	{
		printf("SPI instance is NULL\n");
		return 1;
	}

	if (conf->freq == 0)
	{
		printf("SPI frequency is 0\n");
		return 2;
	}

	if (conf->inst == spi0)
	{
		switch (conf->miso)
		{
		case 0:
		case 4:
		case 16:
			break;
		default:
			printf("Invalid MISO pin for SPI0\n");
			return 3;
		}

		switch (conf->mosi)
		{
		case 3:
		case 7:
		case 19:
			break;
		default:
			printf("Invalid MOSI pin for SPI0\n");
			return 4;
		}

		switch (conf->sck)
		{
		case 2:
		case 6:
		case 17:
			break;
		default:
			printf("Invalid SCK pin for SPI0\n");
			return 5;
		}
	}
	else if (conf->inst == spi1)
	{
		switch (conf->miso)
		{
		case 8:
		case 12:
			break;
		default:
			printf("Invalid MISO pin for SPI1\n");
			return 3;
		}

		switch (conf->mosi)
		{
		case 11:
		case 15:
			break;
		default:
			printf("Invalid MOSI pin for SPI1\n");
			return 4;
		}

		switch (conf->sck)
		{
		case 10:
		case 14:
			break;
		default:
			printf("Invalid SCK pin for SPI1\n");
			return 5;
		}
	}
	else
	{
		printf("Invalid SPI instance\n");
		return 6;
	}

	return 0;
}

int hardware_cs(spi_conf_t *conf)
{
	if (conf->inst == spi0)
	{
		switch (conf->cs)
		{
		case 1:
		case 5:
		case 17:
			return true;
		default:
			return false;
		}
	}
	else if (conf->inst == spi1)
	{
		switch (conf->cs)
		{
		case 9:
		case 13:
			return true;
		default:
			return false;
		}
	}
	return false;
}

#endif