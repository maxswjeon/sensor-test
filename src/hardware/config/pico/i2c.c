#ifdef PICO_BUILD

#include <pico/stdlib.h>
#include <hardware/config/i2c.h>

int setup_i2c(i2c_conf_t *conf)
{
	int valid = validate_i2c(conf);
	if (valid != 0)
	{
		return valid;
	}

	i2c_init(conf->inst, conf->freq);
	gpio_set_function(conf->sda, GPIO_FUNC_I2C);
	gpio_set_function(conf->scl, GPIO_FUNC_I2C);
	gpio_pull_up(conf->sda);
	gpio_pull_up(conf->scl);
}

uint8_t validate_i2c(i2c_conf_t *conf)
{
	if (conf->inst == NULL)
	{
		printf("I2C instance is NULL\n");
		return 1;
	}

	if (conf->freq == 0)
	{
		printf("I2C frequency is 0\n");
		return 2;
	}

	if (conf->inst == i2c0)
	{
		switch (conf->sda)
		{
		case 0:
		case 4:
		case 8:
		case 12:
		case 16:
		case 20:
			break;
		default:
			printf("Invalid SDA pin for I2C0\n");
			return 3;
		}

		switch (conf->scl)
		{
		case 1:
		case 5:
		case 9:
		case 13:
		case 17:
		case 21:
			break;
		default:
			printf("Invalid SCL pin for I2C0\n");
			return 4;
		}
	}

	if (conf->inst == i2c1)
	{
		switch (conf->sda)
		{
		case 2:
		case 6:
		case 10:
		case 14:
		case 18:
		case 26:
			break;
		default:
			printf("Invalid SDA pin for I2C1\n");
			return 3;
		}

		switch (conf->scl)
		{
		case 3:
		case 7:
		case 11:
		case 15:
		case 19:
		case 27:
			break;
		default:
			printf("Invalid SCL pin for I2C1\n");
			return 4;
		}
	}
	else
	{
		printf("Invalid I2C instance\n");
		return 5;
	}

	return 0;
}

#endif