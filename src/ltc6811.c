#include "ltc6811.h"
#include <string.h>

// NOTE: Command codes and subcommands must match datasheet
// The below values are placeholders and should be verified against the LTC6811 datasheet.
// They mirror common Linduino definitions for LTC6811/LTC6804 families.

#define CMD_WRCFGA  0x0001u // placeholder
#define CMD_RDCFGA  0x0002u // placeholder
#define CMD_WRCFGB  0x0003u // placeholder
#define CMD_RDCFGB  0x0004u // placeholder
#define CMD_ADCV    0x0260u // Start cell voltage ADC, normal mode placeholder
#define CMD_ADAX    0x0460u // Start AUX (GPIO) ADC placeholder
#define CMD_RDCVA   0x0005u // read cell voltages group A placeholder
#define CMD_RDCVB   0x0006u
#define CMD_RDCVC   0x0007u
#define CMD_RDCVD   0x0008u
#define CMD_RDAUXA  0x0009u
#define CMD_RDAUXB  0x000Au
#define CMD_POLLADC 0x07E4u // PLADC placeholder

#define BYTES_IN_REG_GROUP 6

// PEC15 table based on polynomial 0x4599
static uint16_t pec15_table[256];
static bool pec_table_init = false;

static void pec15_init_table(void) {
	if (pec_table_init) return;
	for (uint16_t i = 0; i < 256; i++) {
		uint16_t remainder = i << 7;
		for (uint8_t bit = 0; bit < 8; bit++) {
			if (remainder & 0x4000) {
				remainder = (remainder << 1) ^ 0x4599;
			} else {
				remainder = (remainder << 1);
			}
		}
		pec15_table[i] = remainder & 0xFFFFu;
	}
	pec_table_init = true;
}

uint16_t ltc6811_pec15(const uint8_t *data, size_t len) {
	pec15_init_table();
	uint16_t remainder = 16; // PEC15 seed
	for (size_t i = 0; i < len; i++) {
		uint16_t addr = ((remainder >> 7) ^ data[i]) & 0xFFu;
		remainder = (remainder << 8) ^ pec15_table[addr];
	}
	return remainder << 1; // The PEC15 has 15 bits; Linduino shifts left by 1
}

static void append_pec(uint8_t *buf, size_t len_without_pec) {
	uint16_t pec = ltc6811_pec15(buf, len_without_pec);
	buf[len_without_pec] = (uint8_t)(pec >> 8);
	buf[len_without_pec + 1] = (uint8_t)(pec & 0xFF);
}

static void build_cmd(uint16_t cmd, uint8_t tx[4]) {
	tx[0] = (uint8_t)(cmd >> 8);
	tx[1] = (uint8_t)(cmd & 0xFF);
	uint16_t pec = ltc6811_pec15(tx, 2);
	tx[2] = (uint8_t)(pec >> 8);
	tx[3] = (uint8_t)(pec & 0xFF);
}

void ltc6811_init(ltc6811_bus_t *bus, const ltc_hal_t *hal, uint8_t num_devices) {
	bus->hal = hal;
	bus->num_devices = num_devices;
}

static void wakeup_pulse(const ltc6811_bus_t *bus, uint32_t us) {
	if (bus->hal->cs_assert) bus->hal->cs_assert();
	// Send idle bytes at low speed via LTC6820 by SPI clocking
	uint8_t dummy = 0xFF;
	uint8_t rx;
	bus->hal->spi_transfer(&dummy, &rx, 1, 1);
	if (bus->hal->delay_us) bus->hal->delay_us(us);
	if (bus->hal->cs_deassert) bus->hal->cs_deassert();
}

void ltc6811_wakeup_idle(const ltc6811_bus_t *bus) {
	wakeup_pulse(bus, 300);
}

void ltc6811_wakeup_sleep(const ltc6811_bus_t *bus) {
	wakeup_pulse(bus, 1000);
}

// Serialize CFGA for one device into 6 bytes (no PEC)
static void serialize_cfga(const ltc6811_config_t *cfg, uint8_t out[6]) {
	memset(out, 0, 6);
	// Map fields into 6 register bytes per datasheet. Placeholder mapping.
	// Byte 0: GPIO bits and REFON
	if (cfg->gpio_pullup_bitmap & 0x01) out[0] |= (1u << 0);
	if (cfg->gpio_pullup_bitmap & 0x02) out[0] |= (1u << 1);
	if (cfg->gpio_pullup_bitmap & 0x04) out[0] |= (1u << 2);
	if (cfg->gpio_pullup_bitmap & 0x08) out[0] |= (1u << 3);
	if (cfg->gpio_pullup_bitmap & 0x10) out[0] |= (1u << 4);
	// Byte 1: ADC mode, DCP
	out[1] = ((uint8_t)cfg->adc_mode & 0x03) << 6;
	if (cfg->discharge_permitted == LTC6811_DCP_ENABLED) out[1] |= (1u << 4);
	// UV/OV thresholds are device specific encoding; placeholder scales
	uint16_t uv = cfg->uv_threshold_mv / 16;
	uint16_t ov = cfg->ov_threshold_mv / 16;
	out[2] = (uint8_t)(uv & 0xFF);
	out[3] = (uint8_t)(((uv >> 8) & 0x0F) | ((ov & 0x0F) << 4));
	out[4] = (uint8_t)((ov >> 4) & 0xFF);
	// Discharge timeout placeholder
	out[5] = (uint8_t)(cfg->discharge_timeout_s & 0xFF);
}

int ltc6811_write_config(const ltc6811_bus_t *bus, const ltc6811_config_t *cfg_per_device) {
	uint8_t cmd[4];
	build_cmd(CMD_WRCFGA, cmd);
	// Each device gets 6 data bytes + 2 PEC
	size_t frame_len = 4 + (bus->num_devices * (6 + 2));
	uint8_t tx[4 + LTC6811_MAX_DEVICES * 8];
	uint8_t rx[sizeof(tx)];
	memcpy(tx, cmd, 4);
	for (uint8_t i = 0; i < bus->num_devices; i++) {
		uint8_t *p = &tx[4 + i * 8];
		serialize_cfga(&cfg_per_device[i], p);
		append_pec(p, 6);
	}
	ltc6811_wakeup_idle(bus);
	int rc = bus->hal->spi_transfer(tx, rx, frame_len, 10);
	return rc;
}

int ltc6811_read_config(const ltc6811_bus_t *bus, ltc6811_config_t *cfg_out) {
	uint8_t cmd[4];
	build_cmd(CMD_RDCFGA, cmd);
	// Expect back 6 bytes + 2 PEC per device
	size_t rx_len = (bus->num_devices * (6 + 2));
	uint8_t rx[4 + LTC6811_MAX_DEVICES * 8];
	uint8_t tx[sizeof(rx)];
	memset(tx, 0x00, sizeof(tx));
	memcpy(tx, cmd, 4);
	ltc6811_wakeup_idle(bus);
	int rc = bus->hal->spi_transfer(tx, rx, 4 + rx_len, 10);
	if (rc) return rc;
	for (uint8_t i = 0; i < bus->num_devices; i++) {
		const uint8_t *p = &rx[4 + i * 8];
		uint16_t pec_calc = ltc6811_pec15(p, 6);
		uint16_t pec_rx = ((uint16_t)p[6] << 8) | p[7];
		if (pec_calc != pec_rx) return -2;
		// TODO: deserialize back into cfg_out[i]
	}
	return 0;
}

int ltc6811_start_cell_adc(const ltc6811_bus_t *bus) {
	uint8_t cmd[4];
	build_cmd(CMD_ADCV, cmd);
	uint8_t rx[4];
	ltc6811_wakeup_idle(bus);
	return bus->hal->spi_transfer(cmd, rx, 4, 10);
}

int ltc6811_start_gpio_adc(const ltc6811_bus_t *bus) {
	uint8_t cmd[4];
	build_cmd(CMD_ADAX, cmd);
	uint8_t rx[4];
	ltc6811_wakeup_idle(bus);
	return bus->hal->spi_transfer(cmd, rx, 4, 10);
}

bool ltc6811_poll_adc_complete(const ltc6811_bus_t *bus, uint32_t timeout_ms) {
	uint8_t cmd[4];
	build_cmd(CMD_POLLADC, cmd);
	uint8_t rx[4];
	uint32_t start = bus->hal->millis ? bus->hal->millis() : 0;
	for (;;) {
		ltc6811_wakeup_idle(bus);
		if (bus->hal->spi_transfer(cmd, rx, 4, 10) == 0) {
			// The returned pattern indicates busy/done; placeholder assumes done
			return true;
		}
		if (bus->hal->millis && (bus->hal->millis() - start) > timeout_ms) return false;
		if (bus->hal->delay_ms) bus->hal->delay_ms(1);
	}
}

static int read_cell_group(const ltc6811_bus_t *bus, uint16_t cmd, uint16_t out_mv[LTC6811_MAX_DEVICES][3]) {
	uint8_t cmdbuf[4];
	build_cmd(cmd, cmdbuf);
	// Each device returns 6 bytes + 2 PEC representing 3 voltages (2 bytes each)
	size_t rx_len = (bus->num_devices * 8);
	uint8_t rx[4 + LTC6811_MAX_DEVICES * 8];
	uint8_t tx[sizeof(rx)];
	memset(tx, 0x00, sizeof(tx));
	memcpy(tx, cmdbuf, 4);
	ltc6811_wakeup_idle(bus);
	int rc = bus->hal->spi_transfer(tx, rx, 4 + rx_len, 20);
	if (rc) return rc;
	for (uint8_t i = 0; i < bus->num_devices; i++) {
		const uint8_t *p = &rx[4 + i * 8];
		uint16_t pec_calc = ltc6811_pec15(p, 6);
		uint16_t pec_rx = ((uint16_t)p[6] << 8) | p[7];
		if (pec_calc != pec_rx) return -3;
		for (int j = 0; j < 3; j++) {
			uint16_t raw = ((uint16_t)p[j * 2 + 1] << 8) | p[j * 2 + 0];
			out_mv[i][j] = raw; // Scaling to mV per datasheet
		}
	}
	return 0;
}

int ltc6811_read_cell_voltages(const ltc6811_bus_t *bus, ltc6811_measurements_t *meas) {
	uint16_t ga[LTC6811_MAX_DEVICES][3];
	uint16_t gb[LTC6811_MAX_DEVICES][3];
	uint16_t gc[LTC6811_MAX_DEVICES][3];
	uint16_t gd[LTC6811_MAX_DEVICES][3];
	int rc = read_cell_group(bus, CMD_RDCVA, ga); if (rc) return rc;
	rc = read_cell_group(bus, CMD_RDCVB, gb); if (rc) return rc;
	rc = read_cell_group(bus, CMD_RDCVC, gc); if (rc) return rc;
	rc = read_cell_group(bus, CMD_RDCVD, gd); if (rc) return rc;
	for (uint8_t i = 0; i < bus->num_devices; i++) {
		meas[i].cell_mv[0] = ga[i][0];
		meas[i].cell_mv[1] = ga[i][1];
		meas[i].cell_mv[2] = ga[i][2];
		meas[i].cell_mv[3] = gb[i][0];
		meas[i].cell_mv[4] = gb[i][1];
		meas[i].cell_mv[5] = gb[i][2];
		meas[i].cell_mv[6] = gc[i][0];
		meas[i].cell_mv[7] = gc[i][1];
		meas[i].cell_mv[8] = gc[i][2];
		meas[i].cell_mv[9] = gd[i][0];
		meas[i].cell_mv[10] = gd[i][1];
		meas[i].cell_mv[11] = gd[i][2];
	}
	return 0;
}

static int read_aux_group(const ltc6811_bus_t *bus, uint16_t cmd, uint16_t out_mv[LTC6811_MAX_DEVICES][3]) {
	return read_cell_group(bus, cmd, out_mv);
}

int ltc6811_read_gpio_aux(const ltc6811_bus_t *bus, ltc6811_measurements_t *meas) {
	uint16_t aa[LTC6811_MAX_DEVICES][3];
	uint16_t ab[LTC6811_MAX_DEVICES][3];
	int rc = read_aux_group(bus, CMD_RDAUXA, aa); if (rc) return rc;
	rc = read_aux_group(bus, CMD_RDAUXB, ab); if (rc) return rc;
	for (uint8_t i = 0; i < bus->num_devices; i++) {
		meas[i].aux_mv[0] = aa[i][0];
		meas[i].aux_mv[1] = aa[i][1];
		meas[i].aux_mv[2] = aa[i][2];
		meas[i].aux_mv[3] = ab[i][0];
		meas[i].aux_mv[4] = ab[i][1];
		meas[i].aux_mv[5] = ab[i][2];
	}
	return 0;
}