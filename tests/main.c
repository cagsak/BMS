#include <stdio.h>
#include <string.h>
#include "mock_hal.h"
#include "bms_master.h"

int main(void) {
	const ltc_hal_t *hal = mock_hal();
	bms_master_t master;
	uint8_t num_slaves = 2;
	bms_master_init(&master, hal, num_slaves);

	ltc6811_config_t cfg[2];
	memset(cfg, 0, sizeof(cfg));
	for (int i = 0; i < num_slaves; i++) {
		cfg[i].adc_mode = LTC6811_ADCMODE_7k;
		cfg[i].discharge_permitted = LTC6811_DCP_DISABLED;
		cfg[i].uv_threshold_mv = 2500;
		cfg[i].ov_threshold_mv = 4200;
		cfg[i].discharge_timeout_s = 0;
		cfg[i].gpio_pullup_bitmap = 0x1F;
		for (int t = 0; t < BMS_NTC_PER_SLAVE; t++) {
			master.slave_params[i].ntc_params[t].v_supply_mv = 3300;
			master.slave_params[i].ntc_params[t].r_fixed_ohm = 10000;
			master.slave_params[i].ntc_params[t].r25_ohm = 10000;
			master.slave_params[i].ntc_params[t].beta_k = 3435;
		}
	}

	int rc = bms_master_configure(&master, cfg);
	printf("configure rc=%d\n", rc);

	ltc6811_measurements_t meas_cells[2];
	memset(meas_cells, 0, sizeof(meas_cells));
	rc = bms_master_sample_cells(&master, meas_cells);
	printf("cells rc=%d first_cell0=%u mV\n", rc, (unsigned)meas_cells[0].cell_mv[0]);

	ltc6811_measurements_t aux_raw[2];
	int16_t temps[BMS_MAX_SLAVES][BMS_NTC_PER_SLAVE];
	memset(aux_raw, 0, sizeof(aux_raw));
	rc = bms_master_sample_temperatures(&master, temps, aux_raw);
	printf("temps rc=%d t0=%d c10\n", rc, temps[0][0]);

	return 0;
}