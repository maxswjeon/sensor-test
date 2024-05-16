#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/bootrom.h>
#include <hardware/spi.h>
#include <hardware/watchdog.h>

// #include <firmware/BHI360.h>
// #define FIRMWARE BHI360_fw
// #define FIRMWARE_LEN BHI360_fw_len

// #include <firmware/Bosch_Shuttle3_BHI360.h>
// #define FIRMWARE Bosch_Shuttle3_BHI360_fw
// #define FIRMWARE_LEN Bosch_Shuttle3_BHI360_fw_len

#include <firmware/Bosch_Shuttle3_BHI360_Turbo.h>
#define FIRMWARE Bosch_Shuttle3_BHI360_Turbo_fw
#define FIRMWARE_LEN Bosch_Shuttle3_BHI360_Turbo_fw_len

#define UART_BUFFER_SIZE 256

#define SPI_INST spi0
#define SPI_MISO 0
#define SPI_CSn 1
#define SPI_SCK 2
#define SPI_MOSI 3

#define SENSOR_INTR 4

auto_init_mutex(ibuf_mutex);
int16_t ibuf[UART_BUFFER_SIZE];
uint16_t ibuf_rhand = 0;
uint16_t ibuf_whand = 0;

#define WAIT_TRUE 0
#define WAIT_FALSE 1
#define WAIT_NONE 2

typedef struct
{
	uint16_t length;
	uint8_t data[512];
} fifo_data_t;

#define FIFO_BUFFER_SIZE 64

typedef struct
{
	uint16_t rindex;
	uint16_t windex;
	fifo_data_t data[FIFO_BUFFER_SIZE];
} fifo_buffer_t;

fifo_buffer_t fifo_data[3];

void clear_ibuf(uint64_t timeout_us = 0)
{
	mutex_enter_timeout_us(&ibuf_mutex, timeout_us);
	ibuf_whand = ibuf_rhand = 0;
	mutex_exit(&ibuf_mutex);
}

void spi_setup()
{
	spi_init(SPI_INST, 1000 * 1000);
	spi_set_format(SPI_INST, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST); // MSB First Diagram with Little Endian
	gpio_set_function(SPI_MISO, GPIO_FUNC_SPI);
	gpio_set_function(SPI_SCK, GPIO_FUNC_SPI);
	gpio_set_function(SPI_MOSI, GPIO_FUNC_SPI);

	gpio_init(SPI_CSn);
	gpio_set_dir(SPI_CSn, GPIO_OUT);
	gpio_put(SPI_CSn, 1);
}

uint8_t read_reg(uint8_t address)
{
	uint8_t buffer;

	address |= 0x80; // Read command
	gpio_put(SPI_CSn, 0);
	spi_write_blocking(SPI_INST, &address, 1);
	spi_read_blocking(SPI_INST, 0, &buffer, 1);
	gpio_put(SPI_CSn, 1);
	return buffer;
}

void write_reg(uint8_t address, uint8_t value)
{
	gpio_put(SPI_CSn, 0);
	spi_write_blocking(SPI_INST, &address, 1);
	spi_write_blocking(SPI_INST, &value, 1);
	gpio_put(SPI_CSn, 1);
}

void write_data(uint8_t reg, uint8_t *data, uint8_t len)
{
	uint8_t buffer[257];
	buffer[0] = reg;
	memcpy(&buffer[1], data, len);

	gpio_put(SPI_CSn, 0);
	spi_write_blocking(SPI_INST, buffer, len + 1);
	gpio_put(SPI_CSn, 1);
}

uint8_t intr_status()
{
	uint8_t status = read_reg(0x2D);

	// uint8_t wakeup_status = (status >> 1) & 0x03;
	// uint8_t nwakeup_status = (status >> 3) & 0x03;

	// printf("[INFO] Host Interrupt: %s\n", status & 0x01 ? "Active" : "Inactive");

	// if (wakeup_status == 0)
	// {
	// 	printf("[INFO] Wake-up Status: No Data\n");
	// }
	// else if (wakeup_status == 1)
	// {
	// 	printf("[INFO] Wake-up Status: Immediate\n");
	// }
	// else if (wakeup_status == 2)
	// {
	// 	printf("[INFO] Wake-up Status: Latency\n");
	// }
	// else
	// {
	// 	printf("[INFO] Wake-up Status: Watermark\n");
	// }

	// if (nwakeup_status == 0)
	// {
	// 	printf("[INFO] Non-Wake-up Status: No Data\n");
	// }
	// else if (nwakeup_status == 1)
	// {
	// 	printf("[INFO] Non-Wake-up Status: Immediate\n");
	// }
	// else if (nwakeup_status == 2)
	// {
	// 	printf("[INFO] Non-Wake-up Status: Latency\n");
	// }
	// else
	// {
	// 	printf("[INFO] Non-Wake-up Status: Watermark\n");
	// }

	// printf("[INFO] Status & Data Status: %s\n", status & 0x20 ? "Non-Empty" : "Empty");
	// printf("[INFO] Async Response: %s\n", status & 0x40 ? "True" : "False");
	// printf("[INFO] Fatal Error: %s\n", status & 0x80 ? "True" : "False");
	// printf("\n");

	return status;
}

uint16_t get_fifo_data_size(uint8_t index)
{
	uint16_t size = 0;
	size = read_reg(index);
	size |= read_reg(index) << 8;

	return size;
}

void print_status(uint8_t status)
{
	bool interface_ready = (status & 0x10) != 0;
	bool firmware_loading = (status & 0x20) == 0;
	bool firmware_verify = (status & 0x40) == 0;
	bool firmware_running = (status & 0x80) == 0;

	printf("[INFO] Boot Status: 0x%02X\n", status);
	printf("[INFO] \tInterface: %s\n", interface_ready ? "Ready" : "Not Ready");
	if (firmware_loading)
	{
		printf("[INFO] \tFirmware: Verification in Progress\n");
	}
	else if (firmware_verify)
	{
		printf("[INFO] \tFirmware: Verification Success\n");
	}
	else
	{
		printf("[INFO] \tFirmware: Verification Failed\n");
	}
	printf("[INFO] \tFirmware State: %s\n\n", firmware_running ? "Running" : "Halted");
}

bool poll_status(uint8_t check_ready, uint8_t check_verify, uint64_t print_interval_ms = 1000, uint64_t timeout_ms = 5000)
{
	uint64_t time_last_print = 0;
	uint64_t time_check_start = time_us_64();
	while (true)
	{
		uint8_t status = read_reg(0x25);
		bool interface_ready = (status & 0x10) != 0;
		bool firmware_loading = (status & 0x20) == 0;
		bool firmware_verify = (status & 0x40) == 0;
		bool firmware_running = (status & 0x80) == 0;

		bool verified = !firmware_loading && firmware_verify;

		if (time_us_64() - time_last_print > print_interval_ms * 1000)
		{
			print_status(status);
			time_last_print = time_us_64();
		}

		if (time_us_64() - time_check_start > timeout_ms * 1000)
		{
			printf("[ERROR] Timeout while waiting for the firmware to load\n");
			return false;
		}

		if (check_verify != WAIT_NONE && ((check_verify == WAIT_TRUE) == verified))
		{
			print_status(status);
			return true;
		}

		if (check_ready != WAIT_NONE && ((check_ready == WAIT_TRUE) == interface_ready))
		{
			print_status(status);
			return true;
		}
	}
}

void pause()
{
	while (true)
	{
		printf("Press any key to continue...\n");

		bool skip = false;
		mutex_enter_timeout_us(&ibuf_mutex, 1000);
		if (ibuf_rhand != ibuf_whand)
		{
			ibuf_rhand++; // Pop the character
			skip = true;
		}
		mutex_exit(&ibuf_mutex);

		if (skip)
			break;
		sleep_ms(1000);
	}
}

#define CHANNEL_WAKEUP_FIFO 0x01
#define CHANNEL_NON_WAKEUP_FIFO 0x02
#define CHANNEL_STATUS_DEBUG_FIFO 0x03

void process_fifo(uint8_t fifo_index)
{
	fifo_buffer_t *buffer = fifo_data + (fifo_index - 1);

	uint32_t read_length = 0;

	fifo_index = fifo_index | 0x80; // Read command

	uint8_t data_size_buf[2];
	gpio_put(SPI_CSn, 0);
	spi_write_blocking(SPI_INST, &fifo_index, 1);
	spi_read_blocking(SPI_INST, 0, data_size_buf, 2);
	gpio_put(SPI_CSn, 1);

	uint16_t data_size = data_size_buf[0] | (data_size_buf[1] << 8);
	if (data_size == 0)
	{
		return;
	}

	uint16_t data_read = 0;
	while (data_read < data_size)
	{
		uint16_t remainder = data_size - data_read;
		buffer->data[buffer->windex].length = remainder > 512 ? 512 : remainder;

		gpio_put(SPI_CSn, 0);
		spi_write_blocking(SPI_INST, &fifo_index, 1);
		spi_read_blocking(SPI_INST, 0, buffer->data[buffer->windex].data, buffer->data[buffer->windex].length);
		gpio_put(SPI_CSn, 1);

		data_read += buffer->data[buffer->windex].length;
		buffer->windex = (buffer->windex + 1) % FIFO_BUFFER_SIZE;
	}
}

int sensor_task()
{
	uint8_t messages = 0;

	uint8_t intr = intr_status();

	if (intr & 0x01 == 0)
	{
		return 0;
	}

	uint8_t wakeup_status = (intr >> 1) & 0x03;
	uint8_t nwakeup_status = (intr >> 3) & 0x03;
	uint8_t debug_status = (intr >> 5) & 0x03;

	if (wakeup_status != 0)
	{
		process_fifo(CHANNEL_WAKEUP_FIFO);
		messages++;
	}

	if (nwakeup_status != 0)
	{
		process_fifo(CHANNEL_NON_WAKEUP_FIFO);
		messages++;
	}

	if (debug_status != 0)
	{
		process_fifo(CHANNEL_STATUS_DEBUG_FIFO);
		messages++;
	}

	return messages;
}

void sensor_main()
{
	pause();
	spi_setup();

	printf("[INFO] Host Interrupt State: %s\n", gpio_get(SENSOR_INTR) ? "High" : "Low");

	printf("[INFO] Issuing a Sensor Reset\n");
	write_reg(0x14, 0x01);
	sleep_ms(100);

	// Read the sensor ID
	uint8_t sensor_id = read_reg(0x2B);
	while (sensor_id != 0x7A)
	{
		printf("[ERROR] Sensor ID is not correct, expected 0x7A, got 0x%02X\n", sensor_id);
		sleep_ms(1000);
		sensor_id = read_reg(0x2B);
	}
	printf("[INFO] Sensor ID: 0x%02X\n", sensor_id);

	uint8_t fuser2_identifier = read_reg(0x1C);
	uint8_t fuser2_revision = read_reg(0x1D);
	uint8_t rom_revision_lsb = read_reg(0x1E);
	uint8_t rom_revision_msb = read_reg(0x1F);

	printf("[INFO] Fuser2 Identifier: 0x%02X\n", fuser2_identifier);
	printf("[INFO] Fuser2 Revision: 0x%02X\n", fuser2_revision);
	printf("[INFO] ROM Revision: 0x%02X%02X\n", rom_revision_msb, rom_revision_lsb);
	printf("\n");

	printf("[INFO] Issuing a Sensor Reset\n");
	write_reg(0x14, 0x01);
	sleep_ms(100);

	printf("[INFO] Setting Sensor Configuration\n");

	printf("[INFO] Setting Turbo Mode\n");
	write_reg(0x05, 0x00);
	printf("\n");

	printf("[INFO] Setting Host Interrupt");
	write_reg(0x07, 0x00);
	printf("\n");

	printf("[INFO] Setting Host Interface\n");
	write_reg(0x06, 0x00);
	printf("\n");

	if (!poll_status(WAIT_TRUE, WAIT_NONE))
	{
		printf("[ERRR] Interface is not ready\n");
		printf("[ERRR] Please unplug and replug the sensor\n");
		return;
	}

	printf("[INFO] Loading Firmware\n");
	uint8_t payload[5];

	payload[0] = 0x00; // ?? 시발 왜 0x00이지
	payload[1] = 0x02;
	payload[2] = 0x00; // 거꾸로 넣어야함? 왜????

	uint16_t fw_len = FIRMWARE_LEN / 4;
	payload[3] = fw_len & 0xFF;
	payload[4] = (fw_len >> 8) & 0xFF;

	printf("[INFO] Command Length: 5\n");
	printf("[INFO] Firmware Length: %d\n", FIRMWARE_LEN);
	printf("[INFO] Total Length: %d\n", 5 + FIRMWARE_LEN);
	printf("\n");

	gpio_put(SPI_CSn, 0);
	spi_write_blocking(SPI_INST, payload, 5);
	spi_write_blocking(SPI_INST, FIRMWARE, FIRMWARE_LEN);
	gpio_put(SPI_CSn, 1);

	printf("[INFO] Verifying Firmware\n");
	poll_status(WAIT_NONE, WAIT_TRUE, 1000, 60 * 1000);

	printf("[INFO] Starting Firmware\n");
	payload[0] = 0x00;
	payload[1] = 0x03;
	payload[2] = 0x00;
	payload[3] = 0x00;
	payload[4] = 0x00;

	gpio_put(SPI_CSn, 0);
	spi_write_blocking(SPI_INST, payload, 5);
	gpio_put(SPI_CSn, 1);

	poll_status(WAIT_FALSE, WAIT_NONE);
	poll_status(WAIT_TRUE, WAIT_NONE);

	uint8_t initalized = 0;
	while (initalized < 2)
	{
		initalized += sensor_task();
	}

	sleep_ms(100);
	printf("\n[INFO] Sensor Initialized\n\n");
	sleep_ms(100);

	float rate = 100;

	uint8_t sensor_init[13];
	sensor_init[0] = 0x00;

	sensor_init[1] = 0x0D; // 0x00
	sensor_init[2] = 0x00; // 0x01

	sensor_init[3] = 0x08; // 0x02
	sensor_init[4] = 0x00; // 0x03

	sensor_init[5] = 0x2C; // 0x04 BHY2_SENSOR_ID_ORI_WU

	sensor_init[6] = (uint32_t)rate & 0xFF;			// 0x05
	sensor_init[7] = ((uint32_t)rate >> 8) & 0xFF;	// 0x06
	sensor_init[8] = ((uint32_t)rate >> 16) & 0xFF; // 0x07
	sensor_init[9] = ((uint32_t)rate >> 24) & 0xFF; // 0x08

	sensor_init[10] = 0x00; // 0x09
	sensor_init[11] = 0x00; // 0x0A
	sensor_init[12] = 0x00; // 0x0B

	gpio_put(SPI_CSn, 0);
	spi_write_blocking(SPI_INST, sensor_init, 13);
	gpio_put(SPI_CSn, 1);

	while (true)
	{
		sensor_task();
	}

	printf("[WARN] Sensor Core Exited\n");
}

void handle_uart()
{
	int res = getchar_timeout_us(10);
	if (res == PICO_ERROR_TIMEOUT)
	{
		return;
	}

	// Ctrl+C
	if (res == 3)
	{
		printf("[INFO] Ctrl+C received, entering FLASH mode\n");
		multicore_reset_core1();
		reset_usb_boot(0, 0);
		return;
	}

	// Ctrl+R
	if (res == 18)
	{
		printf("[INFO] Ctrl+R received, restarting\n");
		multicore_reset_core1();

		// Reset the sensor
		watchdog_enable(1, 1);
		while (true)
			;
	}

	printf("%c", res);

	mutex_enter_timeout_us(&ibuf_mutex, 1000);
	if (ibuf_rhand == (ibuf_whand + 1) % UART_BUFFER_SIZE)
	{ // If the buffer is full, skip the character
		printf("[WARN] UART Buffer is full\n");
		mutex_exit(&ibuf_mutex);
		return;
	}

	ibuf[ibuf_whand++] = res;
	ibuf_whand %= UART_BUFFER_SIZE;
	mutex_exit(&ibuf_mutex);
}

void print_fifo()
{
	for (int i = 0; i < 3; ++i)
	{
		if (fifo_data[i].rindex == fifo_data[i].windex)
		{
			continue;
		}

		while (fifo_data[i].rindex != fifo_data[i].windex)
		{
			fifo_data_t *data = fifo_data[i].data + fifo_data[i].rindex;

			printf("FIFO %d:\n\tLength: %d\n\t", i, data->length);
			for (int j = 0; j < data->length; ++j)
			{
				printf("%02X ", data->data[j]);
			}
			printf("\n");

			fifo_data[i].rindex = (fifo_data[i].rindex + 1) % FIFO_BUFFER_SIZE;
		}
	}
}

int main()
{
	stdio_init_all();

	printf("[INFO] Starting main core\n");

	gpio_init(25);
	gpio_set_dir(25, GPIO_OUT);

	gpio_init(SENSOR_INTR);
	gpio_set_dir(SENSOR_INTR, GPIO_IN);

	multicore_launch_core1(sensor_main);

	uint64_t last_blink = 0;
	bool state = false;

	while (true)
	{
		if (time_us_64() - last_blink > 500 * 1000)
		{
			gpio_put(25, !state);
			state = !state;
			last_blink = time_us_64();
		}

		handle_uart();
		print_fifo(); // Time critical loop, only print 3 FIFO data per loop
	}
}
