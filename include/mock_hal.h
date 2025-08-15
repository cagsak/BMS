#ifndef MOCK_HAL_H
#define MOCK_HAL_H

#include "ltc_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

const ltc_hal_t *mock_hal(void);

#ifdef __cplusplus
}
#endif

#endif // MOCK_HAL_H