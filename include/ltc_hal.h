#ifndef LTC_HAL_H
#define LTC_HAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Portable HAL abstraction to decouple STM32 HAL specifics
// Implement these for your platform (STM32: wrap HAL_SPI_TransmitReceive etc.)

typedef struct {
	// Transmit then receive in a single full-duplex SPI operation
	// Returns 0 on success
	int (*spi_transfer)(const uint8_t *tx, uint8_t *rx, size_t len, uint32_t timeout_ms);

	// Control the LTC6820 CS (if required). Some designs tie CS low; keep optional.
	void (*cs_assert)(void);
	void (*cs_deassert)(void);

	// Microsecond delay (for wakeup pulses timing)
	void (*delay_us)(uint32_t us);

	// Millisecond delay
	void (*delay_ms)(uint32_t ms);

	// Timestamp in milliseconds for simple timeouts
	uint32_t (*millis)(void);
} ltc_hal_t;

#endif // LTC_HAL_H