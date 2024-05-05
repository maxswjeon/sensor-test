#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pico/stdlib.h>
#include <pico/bootrom.h>
#include <hardware/i2c.h>

// #include <firmware/BHI360.h>
#include <firmware/BHI360_cmd.h>

#define I2C_INST i2c0
#define I2C_SDA 0
#define I2C_SCL 1

#define SENSOR_ADDR 0x29

void i2c_setup()
{
	i2c_init(I2C_INST, 1600 * 1000);
	gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
	gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
	gpio_pull_up(I2C_SDA);
	gpio_pull_up(I2C_SCL);
}

void i2c_read(uint8_t address, uint8_t reg, uint8_t *data, size_t len)
{
	i2c_write_blocking(i2c0, address, &reg, 1, true);
	i2c_read_blocking(i2c0, address, data, len, false);
}

void i2c_write(uint8_t address, uint8_t reg, uint8_t *data, size_t len)
{
	uint8_t *buf = (uint8_t *)malloc(len + 1);
	buf[0] = reg;
	memcpy(buf + 1, data, len);
	i2c_write_blocking(i2c0, address, buf, len + 1, false);
	free(buf);
}

void reset_sensor()
{
	uint8_t data = 0x01;
	i2c_write(SENSOR_ADDR, 0x14, &data, 1);

	sleep_ms(100);
}

void setup_interrupt()
{
	uint8_t data = 0x01;
	i2c_write(SENSOR_ADDR, 0x15, &data, 1);
}

uint8_t chip_id()
{
	uint8_t data;
	i2c_read(SENSOR_ADDR, 0x2B, &data, 1);
	return data;
}

void set_turbo(bool enable)
{
	uint8_t data = enable ? 0x00 : 0x01;
	i2c_write(SENSOR_ADDR, 0x05, &data, 1);
}

uint8_t get_boot_status()
{
	uint8_t data;
	i2c_read(SENSOR_ADDR, 0x25, &data, 1);
	return data;
}

void pause()
{
	while (true)
	{
		int read = getchar_timeout_us(1000);
		if (read != PICO_ERROR_TIMEOUT)
		{
			while (read != PICO_ERROR_TIMEOUT)
			{
				printf("%c", read);
				read = getchar_timeout_us(10);
			}
			return;
		}
		printf("Press any key to continue...\n");
		sleep_ms(1000);
	}
}

void poll_boot_status(bool check_ready, bool check_verify, uint64_t print_ms = 1000, uint64_t max_tries = 10000)
{
	uint64_t last = 0;
	for (uint64_t i = 0; i < max_tries; i++)
	{
		uint8_t status = get_boot_status();

		bool interface = (status & 0x10) != 0;
		bool loading = (status & 0x20) == 0;
		bool verify = (status & 0x40) == 0;
		bool running = (status & 0x80) == 0;

		if (time_us_64() - last > print_ms * 1000)
		{
			printf("Boot Status: 0x%02X\n", status);
			printf("\tInterface: %s\n", (status & 0x10) ? "Ready" : "Not Ready");
			if (loading)
			{
				printf("\tFirmware: Verification in Progress\n");
			}
			else if (verify)
			{
				printf("\tFirmware: Verification Success\n");
			}
			else
			{
				printf("\tFirmware: Verification Failed\n");
			}
			printf("\tFirmware State: %s\n\n", (status & 0x80) ? "Halted" : "Running");
			last = time_us_64();
		}

		if (check_verify && !loading && verify)
		{
			return;
		}

		if (check_ready && interface)
		{
			return;
		}
	}
}

uint8_t get_fuser2_identifier()
{
	uint8_t data;
	i2c_read(SENSOR_ADDR, 0x1C, &data, 1);
	return data;
}

uint8_t get_fuser2_revision()
{
	uint8_t data;
	i2c_read(SENSOR_ADDR, 0x1D, &data, 1);
	return data;
}

uint16_t get_rom_version()
{
	uint8_t data[2];
	i2c_read(SENSOR_ADDR, 0x1E, data, 1);
	i2c_read(SENSOR_ADDR, 0x1F, data + 1, 1);

	uint16_t version = (data[1] << 8) | data[0];
	return version;
}

void upload_firmware(uint8_t *data, size_t len)
{
	// uint8_t buf[32];

	// buf[0] = 0x00;
	// buf[1] = 0x02;

	// // Length Endianess is important
	// buf[2] = (len / 4 >> 8) & 0xFF;
	// buf[3] = (len / 4) & 0xFF;

	// i2c_write_blocking(I2C_INST, SENSOR_ADDR, buf, 4, false);

	// printf("\n");
	// size_t offset = 0;
	// while (offset < len)
	// {
	// 	memset(buf, 0, 32);
	// 	size_t target = len - offset > 32 ? 32 : len - offset;

	// 	memcpy(buf + 2, data + offset, target);
	// 	i2c_write_blocking(I2C_INST, SENSOR_ADDR, buf, target, false);
	// 	offset += target;

	// 	printf("\rUploading Firmware: %d%% (%d bytes / %d bytes)", (offset * 100) / len, offset, len);
	// }
	// printf("\n");

	i2c_write_blocking(I2C_INST, SENSOR_ADDR, data, len, false);
}

int main()
{
	stdio_init_all();

	pause();

	i2c_setup();

	printf("Issuing Sensor Reset\n\n");
	reset_sensor();
	sleep_ms(100);

	uint8_t id = chip_id();
	while (id != 0x7A)
	{
		printf("Error: Invalid Chip ID 0x%02X, expected 0x7A\n", id);
		reset_sensor();
		sleep_ms(100);
		id = chip_id();
	}
	printf("Chip ID: 0x%02X\n\n", id);

	printf("Setting Turbo Mode\n\n");
	set_turbo(true);

	printf("Checking Interface Status\n");
	poll_boot_status(true, false);

	printf("Fuser2 Identifier: 0x%02X\n", get_fuser2_identifier());
	printf("Fuser2 Revision: 0x%02X\n", get_fuser2_revision());
	printf("ROM Version: 0x%04X\n\n", get_rom_version());

	printf("Uploading Firmware\n");
	// upload_firmware(BHI360_fw, BHI360_fw_len);
	upload_firmware(BHI360_cmd, BHI360_cmd_len);

	printf("Checking Firmware Verification\n");
	poll_boot_status(false, true);

	sleep_ms(3000);

	printf("Checking Firmware Verification\n");
	poll_boot_status(false, true);

	reset_usb_boot(0, 0);
}