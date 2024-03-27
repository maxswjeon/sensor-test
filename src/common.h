#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdbool.h>

#include "bosch/bhy2.h"
#include "bosch/bhi3.h"
#include "bosch/bhi3_multi_tap.h"

#include "bosch/bhy2_klio.h"
#include "bosch/bhy2_swim.h"
#include "bosch/bhy2_bsec.h"
#include "bosch/bhy2_head_tracker.h"

#define PIN_BOSCH_SCK 2
#define PIN_BOSCH_MOSI 3
#define PIN_BOSCH_MISO 4
#define PIN_BOSCH_CSn 5
#define PIN_BOSCH_RESET 14
#define PIN_BOSCH_INT 15

#ifdef PC
#ifdef COINES_BRIDGE
#define BHY2_RD_WR_LEN 256 /* Coines bridge maximum read write length */
#else
#define BHY2_RD_WR_LEN 44 /* USB maximum read write length(DD firmware) */
#endif
#else
#define BHY2_RD_WR_LEN 256 /* MCU maximum read write length */
#endif

char *get_coines_error(int16_t rslt);
char *get_api_error(int8_t error_code);
char *get_sensor_error_text(uint8_t sensor_error);
char *get_sensor_name(uint8_t sensor_id);
float get_sensor_default_scaling(uint8_t sensor_id);
char *get_sensor_parse_format(uint8_t sensor_id);
char *get_sensor_axis_names(uint8_t sensor_id);
char *get_klio_error(bhy2_klio_driver_error_state_t error);

void setup_interfaces(bool reset_power, enum bhy2_intf intf);
void close_interfaces(enum bhy2_intf intf);
int8_t bhy2_spi_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t length, void *intf_ptr);
int8_t bhy2_spi_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t length, void *intf_ptr);
int8_t bhy2_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t length, void *intf_ptr);
int8_t bhy2_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t length, void *intf_ptr);
void bhy2_delay_us(uint32_t us, void *private_data);
bool get_interrupt_status(void);

#endif /* _COMMON_H_ */
