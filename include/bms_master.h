#ifndef BMS_MASTER_H
#define BMS_MASTER_H

#include <stdint.h>
#include <stdbool.h>
#include "ltc6811.h"
#include "ntc.h"

#define BMS_MAX_SLAVES  LTC6811_MAX_DEVICES
#define BMS_NTC_PER_SLAVE 5

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	ntc_params_t ntc_params[BMS_NTC_PER_SLAVE];
} bms_slave_params_t;

typedef struct {
	ltc6811_bus_t bus;
	uint8_t num_slaves; // number of LTC6811 devices
	bms_slave_params_t slave_params[BMS_MAX_SLAVES];
} bms_master_t;

void bms_master_init(bms_master_t *m, const ltc_hal_t *hal, uint8_t num_slaves);
int bms_master_configure(const bms_master_t *m, const ltc6811_config_t *cfg_per_slave);
int bms_master_sample_cells(const bms_master_t *m, ltc6811_measurements_t *meas_per_slave);
int bms_master_sample_temperatures(const bms_master_t *m, int16_t temps_c10[BMS_MAX_SLAVES][BMS_NTC_PER_SLAVE], ltc6811_measurements_t *aux_raw);

#ifdef __cplusplus
}
#endif

#endif // BMS_MASTER_H