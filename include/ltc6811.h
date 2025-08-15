#ifndef LTC6811_H
#define LTC6811_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "ltc_hal.h"

#define LTC6811_MAX_DEVICES 16
#define LTC6811_NUM_CELLS 12
#define LTC6811_NUM_AUX  6

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	LTC6811_ADCMODE_27k = 0,   // fastest, 27kHz filter
	LTC6811_ADCMODE_14k = 1,
	LTC6811_ADCMODE_7k  = 2,
	LTC6811_ADCMODE_3k  = 3,   // slowest, 3kHz
} ltc6811_adc_mode_t;

typedef enum {
	LTC6811_DCP_DISABLED = 0,
	LTC6811_DCP_ENABLED  = 1,
} ltc6811_discharge_perm_t;

typedef struct {
	// Configuration A (CFGA)
	ltc6811_adc_mode_t adc_mode;   // ADC conversion mode
	ltc6811_discharge_perm_t discharge_permitted; // DCP bit
	uint16_t uv_threshold_mv;      // Programmable under-voltage in mV
	uint16_t ov_threshold_mv;      // Programmable over-voltage in mV
	uint16_t discharge_timeout_s;  // Discharge timer
	uint16_t gpio_pullup_bitmap;   // Enable pullups for GPIO1..GPIO5
} ltc6811_config_t;

typedef struct {
	uint16_t cell_mv[LTC6811_NUM_CELLS];
	uint16_t aux_mv[LTC6811_NUM_AUX]; // GPIO1..GPIO6 typically
	uint16_t ref_mv; // Vref2 if read
	uint16_t die_temp_c10; // 0.1C units if read
	uint16_t status_raw[3];
} ltc6811_measurements_t;

typedef struct {
	const ltc_hal_t *hal;
	uint8_t num_devices; // number of LTC6811 in daisy chain
} ltc6811_bus_t;

// Initialization
void ltc6811_init(ltc6811_bus_t *bus, const ltc_hal_t *hal, uint8_t num_devices);

// Wake up isoSPI chain via LTC6820 by toggling CS and sending dummy bytes
void ltc6811_wakeup_idle(const ltc6811_bus_t *bus);
void ltc6811_wakeup_sleep(const ltc6811_bus_t *bus);

// Write and read configuration registers for all devices
int ltc6811_write_config(const ltc6811_bus_t *bus, const ltc6811_config_t *cfg_per_device);
int ltc6811_read_config(const ltc6811_bus_t *bus, ltc6811_config_t *cfg_out);

// Start conversions
int ltc6811_start_cell_adc(const ltc6811_bus_t *bus);
int ltc6811_start_gpio_adc(const ltc6811_bus_t *bus);

// Poll conversion complete (or use delay)
bool ltc6811_poll_adc_complete(const ltc6811_bus_t *bus, uint32_t timeout_ms);

// Read back measurements
int ltc6811_read_cell_voltages(const ltc6811_bus_t *bus, ltc6811_measurements_t *meas);
int ltc6811_read_gpio_aux(const ltc6811_bus_t *bus, ltc6811_measurements_t *meas);

// Low-level helpers exposed for testing
uint16_t ltc6811_pec15(const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif // LTC6811_H