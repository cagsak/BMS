#include "ltc_hal.h"
#ifdef STM32_HAL
#include "stm32fxxx_hal.h"

#ifndef LTC_SPI_HANDLE
#error "Define LTC_SPI_HANDLE as an SPI_HandleTypeDef*"
#endif

#ifndef LTC_CS_GPIO_Port
#error "Define LTC_CS_GPIO_Port and LTC_CS_Pin for CS control"
#endif

static int stm32_spi_transfer_impl(const uint8_t *tx, uint8_t *rx, size_t len, uint32_t timeout_ms) {
	HAL_StatusTypeDef st;
	if (tx && rx) {
		st = HAL_SPI_TransmitReceive(LTC_SPI_HANDLE, (uint8_t*)tx, rx, (uint16_t)len, timeout_ms);
	} else if (tx) {
		st = HAL_SPI_Transmit(LTC_SPI_HANDLE, (uint8_t*)tx, (uint16_t)len, timeout_ms);
	} else if (rx) {
		st = HAL_SPI_Receive(LTC_SPI_HANDLE, rx, (uint16_t)len, timeout_ms);
	} else {
		return 0;
	}
	return (st == HAL_OK) ? 0 : -1;
}

static void stm32_cs_assert_impl(void) {
	HAL_GPIO_WritePin(LTC_CS_GPIO_Port, LTC_CS_Pin, GPIO_PIN_RESET);
}

static void stm32_cs_deassert_impl(void) {
	HAL_GPIO_WritePin(LTC_CS_GPIO_Port, LTC_CS_Pin, GPIO_PIN_SET);
}

static void stm32_delay_us_impl(uint32_t us) {
	uint32_t start = __HAL_TIM_GET_COUNTER(&htimX);
	while (((__HAL_TIM_GET_COUNTER(&htimX) - start) & 0xFFFF) < us) { }
}

static void stm32_delay_ms_impl(uint32_t ms) {
	HAL_Delay(ms);
}

static uint32_t stm32_millis_impl(void) {
	return HAL_GetTick();
}

static const ltc_hal_t g_stm32_hal = {
	.spi_transfer = stm32_spi_transfer_impl,
	.cs_assert = stm32_cs_assert_impl,
	.cs_deassert = stm32_cs_deassert_impl,
	.delay_us = stm32_delay_us_impl,
	.delay_ms = stm32_delay_ms_impl,
	.millis = stm32_millis_impl,
};

const ltc_hal_t *ltc_stm32_hal(void) {
	return &g_stm32_hal;
}
#endif // STM32_HAL