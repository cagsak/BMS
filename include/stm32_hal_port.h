#ifndef STM32_HAL_PORT_H
#define STM32_HAL_PORT_H

#include "ltc_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

// Define STM32_HAL to enable the port source
// Define LTC_SPI_HANDLE as (SPI_HandleTypeDef*)
// Define LTC_CS_GPIO_Port and LTC_CS_Pin
// Provide a microsecond timer handle htimX or replace stm32_delay_us_impl

const ltc_hal_t *ltc_stm32_hal(void);

#ifdef __cplusplus
}
#endif

#endif // STM32_HAL_PORT_H