#include "bms_master.h"
#include <string.h>

void bms_master_init(bms_master_t *m, const ltc_hal_t *hal, uint8_t num_slaves) {
	ltc6811_init(&m->bus, hal, num_slaves);
	m->num_slaves = num_slaves;
	memset(m->slave_params, 0, sizeof(m->slave_params));
}

int bms_master_configure(const bms_master_t *m, const ltc6811_config_t *cfg_per_slave) {
	return ltc6811_write_config(&m->bus, cfg_per_slave);
}

int bms_master_sample_cells(const bms_master_t *m, ltc6811_measurements_t *meas_per_slave) {
	int rc = ltc6811_start_cell_adc(&m->bus);
	if (rc) return rc;
	if (!ltc6811_poll_adc_complete(&m->bus, 200)) return -1;
	return ltc6811_read_cell_voltages(&m->bus, meas_per_slave);
}

int bms_master_sample_temperatures(const bms_master_t *m, int16_t temps_c10[BMS_MAX_SLAVES][BMS_NTC_PER_SLAVE], ltc6811_measurements_t *aux_raw) {
	int rc = ltc6811_start_gpio_adc(&m->bus);
	if (rc) return rc;
	if (!ltc6811_poll_adc_complete(&m->bus, 200)) return -1;
	rc = ltc6811_read_gpio_aux(&m->bus, aux_raw);
	if (rc) return rc;
	for (uint8_t s = 0; s < m->num_slaves; s++) {
		for (int i = 0; i < BMS_NTC_PER_SLAVE; i++) {
			uint16_t v_mv = aux_raw[s].aux_mv[i];
			temps_c10[s][i] = ntc_mv_to_c10(v_mv, &m->slave_params[s].ntc_params[i]);
		}
	}
	return 0;
}