#include "mock_hal.h"
#include <string.h>
#include <time.h>
#include <unistd.h>

static int mock_spi_transfer_impl(const uint8_t *tx, uint8_t *rx, size_t len, uint32_t timeout_ms) {
	(void)timeout_ms;
	if (rx && len) {
		memset(rx, 0x00, len);
	}
	return 0;
}

static void mock_cs_assert_impl(void) {
}

static void mock_cs_deassert_impl(void) {
}

static void mock_delay_us_impl(uint32_t us) {
	usleep(us);
}

static void mock_delay_ms_impl(uint32_t ms) {
	usleep(ms * 1000);
}

static uint32_t mock_millis_impl(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint32_t)((ts.tv_sec * 1000ull) + (ts.tv_nsec / 1000000ull));
}

static const ltc_hal_t g_mock_hal = {
	.spi_transfer = mock_spi_transfer_impl,
	.cs_assert = mock_cs_assert_impl,
	.cs_deassert = mock_cs_deassert_impl,
	.delay_us = mock_delay_us_impl,
	.delay_ms = mock_delay_ms_impl,
	.millis = mock_millis_impl,
};

const ltc_hal_t *mock_hal(void) {
	return &g_mock_hal;
}