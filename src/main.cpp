#include <stdio.h>
#include <stdlib.h>

#include <pico/stdlib.h>
#include <pico/bootrom.h>
#include <pico/cyw43_arch.h>
#include <pico/malloc.h>

#include <hardware/spi.h>

#include "bosch/firmware/bhi360/BHI360_BMM350C.fw.h"

#define SPI0_SCK_PIN 2
#define SPI0_MOSI_PIN 3
#define SPI0_MISO_PIN 4
#define SPI0_CS_PIN 5

bool spi_turbo_speed(spi_inst_t *inst)
{
	uint8_t payload[8];
	memset(payload, 0, 8);

	payload[0] = 0x00;
	payload[1] = 0x17;
	payload[2] = 0x02;
	payload[3] = 0x00;
	payload[4] = 0x80;
	payload[5] = 0x00;
	payload[6] = 0x00;
	payload[7] = 0x00;

	spi_write_blocking(inst, payload, 8);

	spi_read_blocking(inst, 0x00, payload, 8);

	bool check[8];
	check[0] = payload[0] == 0x0F;
	check[1] = payload[1] == 0x00;
	check[2] = payload[2] == 0x04;
	check[3] = payload[3] == 0x00;
	check[4] = payload[4] == 0x17;
	check[5] = payload[5] == 0x00;
	check[6] = payload[6] == 0x00;
	check[7] = payload[7] == 0x00;

	return check[0] && check[1] && check[2] && check[3] && check[4] && check[5] && check[6] && check[7];
}

int load_firmware(spi_inst_t *inst, const uint8_t *firmware, uint16_t length)
{
	printf("Firmware length: %d\n", length);

	uint8_t *payload = (uint8_t *)malloc(length + 4);
	memset(payload, 0, length + 4);
	payload[0] = 0x00;
	payload[1] = 0x02;
	payload[2] = (uint8_t)((length >> 8) & 0xFF);
	payload[3] = (uint8_t)(length & 0xFF);
	memcpy(payload + 4, firmware, length);

	spi_write_blocking(inst, payload, length + 4);

	free(payload);

	return 0;
}

int boot_firmware(spi_inst_t *inst)
{
	uint8_t payload[4];
	memset(payload, 0, 4);

	payload[0] = 0x00;
	payload[1] = 0x03;
	payload[2] = 0x00;
	payload[3] = 0x00;

	spi_write_blocking(inst, payload, 4);

	return 0;
}

int self_test(spi_inst_t *inst)
{
	uint8_t payload[8];
	memset(payload, 0, 8);

	payload[0] = 0x00;
	payload[1] = 0x0B;
	payload[2] = 0x00;
	payload[3] = 0x04;
	payload[4] = 0x01;
	payload[5] = 0x00;
	payload[6] = 0x00;
	payload[7] = 0x00;

	spi_write_blocking(inst, payload, 8);

	return 0;
}

void check_error(spi_inst_t *inst)
{
	int index = 0;
	uint8_t resp = 0;
	while (true)
	{
		spi_read_blocking(spi0, 0x00, &resp, 1);

		printf("0x%02X\t", resp);

		if (index++ == 7)
		{
			printf("\n");
			index = 0;
		}

		if (resp == 0x00)
		{
			break;
		}
	}
	printf("\n");
}

void check_error(spi_inst_t *inst, uint32_t length)
{
	uint32_t index = 0;
	uint8_t resp = 0;
	while (index < length)
	{
		spi_read_blocking(spi0, 0x00, &resp, 1);

		printf("0x%02X\t", resp);

		if (index++ % 8 == 7)
		{
			printf("\n");
		}
	}
	printf("\n");
}

int main()
{
	stdio_init_all();

	if (cyw43_arch_init())
	{
		printf("Failed to initialize WiFi module\n");

		reset_usb_boot(0, 0);
		return 0;
	}

	sleep_ms(10000);

	printf("Starting...\n");

	spi_init(spi0, 1 * 1000 * 1000);
	gpio_set_function(SPI0_SCK_PIN, GPIO_FUNC_SPI);
	gpio_set_function(SPI0_MOSI_PIN, GPIO_FUNC_SPI);
	gpio_set_function(SPI0_MISO_PIN, GPIO_FUNC_SPI);

	gpio_init(SPI0_CS_PIN);
	gpio_set_dir(SPI0_CS_PIN, GPIO_OUT);
	gpio_put(SPI0_CS_PIN, 1);

	// Load firmware
	printf("Loading firmware...\n");

	uint64_t length = sizeof(bhy2_firmware_image) / 4;
	load_firmware(spi0, bhy2_firmware_image, (uint8_t)length);
	printf("Firmware loaded\n");
	check_error(spi0, 8);

	// Boot firmware
	printf("Booting firmware...\n");
	boot_firmware(spi0);
	printf("Firmware booted\n");
	check_error(spi0, 8);

	// Self test
	printf("Self test...\n");
	self_test(spi0);
	printf("Self test done\n");
	check_error(spi0, 5);

	while (true)
		;
}