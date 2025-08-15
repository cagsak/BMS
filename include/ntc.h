#ifndef NTC_H
#define NTC_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	// Voltage divider params: NTC in series with fixed resistor
	// adc_mv is the measured voltage across the NTC or fixed resistor depending on wiring
	uint32_t v_supply_mv;
	uint32_t r_fixed_ohm;
	// NTC Beta model parameters
	uint32_t r25_ohm;   // Resistance at 25C
	uint32_t beta_k;    // Beta in Kelvin
} ntc_params_t;

// Convert measured voltage (in mV across NTC) to temperature in 0.1C units
int16_t ntc_mv_to_c10(uint16_t v_ntc_mv, const ntc_params_t *p);

#ifdef __cplusplus
}
#endif

#endif // NTC_H