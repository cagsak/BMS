#include "ntc.h"
#include <math.h>

#ifndef M_LN
#define M_LN(x) log(x)
#endif

static inline double kelvin_from_beta(double r_ohm, double r25_ohm, double beta_k) {
	double invT = (1.0 / (25.0 + 273.15)) + (1.0 / beta_k) * log(r_ohm / r25_ohm);
	return 1.0 / invT;
}

int16_t ntc_mv_to_c10(uint16_t v_ntc_mv, const ntc_params_t *p) {
	if (!p || v_ntc_mv == 0 || v_ntc_mv >= p->v_supply_mv) return -32768;
	double v_ntc = (double)v_ntc_mv / 1000.0;
	double v_s = (double)p->v_supply_mv / 1000.0;
	// Assume divider: Vout across NTC to GND, fixed resistor to Vsup
	double r_ntc = (v_ntc * p->r_fixed_ohm) / (v_s - v_ntc);
	double T_k = kelvin_from_beta(r_ntc, p->r25_ohm, p->beta_k);
	double T_c = T_k - 273.15;
	int16_t c10 = (int16_t)lrint(T_c * 10.0);
	return c10;
}