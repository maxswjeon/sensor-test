#ifndef __BOSCH_COMMON_H__
#define __BOSCH_COMMON_H__

#include <lib/bosch/bhy2.h>

#include <hardware/bhy2.h>

#include <stddef.h>

const char *get_coins_error(int16_t);
const char *get_api_error(int8_t);
const char *get_sensor_error_text(uint8_t sensor_error);
const char *get_sensor_name(uint8_t sensor_id);

void setup_interfaces(bhy2_inst_t *, bool reset_power, enum bhy2_intf intf);
void close_interfaces(bhy2_inst_t *);
int8_t bhy2_spi_read(bhy2_inst_t *, uint8_t reg_addr, uint8_t *reg_data, uint32_t length, void *intf_ptr);
int8_t bhy2_spi_write(bhy2_inst_t *, uint8_t reg_addr, const uint8_t *reg_data, uint32_t length, void *intf_ptr);
int8_t bhy2_i2c_read(bhy2_inst_t *, uint8_t reg_addr, uint8_t *reg_data, uint32_t length, void *intf_ptr);
int8_t bhy2_i2c_write(bhy2_inst_t *, uint8_t reg_addr, const uint8_t *reg_data, uint32_t length, void *intf_ptr);
void bhy2_delay_us(bhy2_inst_t *, uint32_t us, void *private_data);
bool get_interrupt_status(bhy2_inst_t *);

#endif // __BOSCH_COMMON_H__