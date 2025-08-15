/**
 * @file LTC6820_Master.h
 * @brief LTC6820 Isolated SPI Interface Driver for Master BMS
 * @version 1.0
 * @date 2024
 * 
 * The LTC6820 provides isolated SPI communication between the master BMS
 * and slave BMS nodes. This driver handles the communication protocol
 * and data formatting.
 */

#ifndef LTC6820_MASTER_H
#define LTC6820_MASTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* LTC6820 Configuration */
#define LTC6820_MAX_CHANNELS        8       // Maximum number of isolated channels
#define LTC6820_MAX_PACKET_SIZE     64      // Maximum packet size per channel
#define LTC6820_TIMEOUT_MS          100     // Communication timeout
#define LTC6820_RETRY_COUNT         3       // Number of retries on failure

/* LTC6820 Command Codes */
#define LTC6820_CMD_WRITE           0x80    // Write command
#define LTC6820_CMD_READ            0x00    // Read command
#define LTC6820_CMD_BROADCAST       0xFF    // Broadcast to all channels

/* LTC6820 Register Addresses */
#define LTC6820_REG_CHANNEL_CTRL    0x00    // Channel control register
#define LTC6820_REG_CHANNEL_STATUS  0x01    // Channel status register
#define LTC6820_REG_CHANNEL_DATA    0x02    // Channel data register
#define LTC6820_REG_CONFIG          0x03    // Configuration register
#define LTC6820_REG_STATUS          0x04    // Status register

/* Channel Control Bits */
#define LTC6820_CH_ENABLE           0x01    // Enable channel
#define LTC6820_CH_DISABLE          0x00    // Disable channel
#define LTC6820_CH_RESET            0x02    // Reset channel
#define LTC6820_CH_FAST_MODE        0x04    // Fast mode enable
#define LTC6820_CH_ISOLATION_TEST   0x08    // Isolation test enable

/* Status Bits */
#define LTC6820_STATUS_READY        0x01    // Channel ready
#define LTC6820_STATUS_ERROR        0x02    // Communication error
#define LTC6820_STATUS_TIMEOUT      0x04    // Timeout error
#define LTC6820_STATUS_ISOLATION    0x08    // Isolation fault
#define LTC6820_STATUS_OVERVOLTAGE  0x10    // Overvoltage fault
#define LTC6820_STATUS_OVERTEMP     0x20    // Overtemperature fault

/* Error Codes */
typedef enum {
    LTC6820_OK = 0,
    LTC6820_ERROR_TIMEOUT,
    LTC6820_ERROR_COMMUNICATION,
    LTC6820_ERROR_CHANNEL_DISABLED,
    LTC6820_ERROR_ISOLATION_FAULT,
    LTC6820_ERROR_INVALID_CHANNEL,
    LTC6820_ERROR_INVALID_PARAM,
    LTC6820_ERROR_NOT_INITIALIZED
} LTC6820_Status_t;

/* Channel Configuration */
typedef struct {
    uint8_t channel_id;             // Channel ID (0-7)
    bool is_enabled;                // Channel enabled
    bool fast_mode;                 // Fast mode enabled
    uint16_t baud_rate;             // Baud rate for this channel
    uint16_t timeout_ms;            // Timeout for this channel
    uint8_t retry_count;            // Retry count for this channel
} LTC6820_ChannelConfig_t;

/* Channel Status */
typedef struct {
    uint8_t channel_id;             // Channel ID
    bool is_ready;                  // Channel ready for communication
    bool has_error;                 // Channel has error
    uint8_t error_flags;            // Error flags
    uint32_t last_communication;    // Last successful communication
    uint32_t error_count;           // Error count
    uint32_t timeout_count;         // Timeout count
} LTC6820_ChannelStatus_t;

/* LTC6820 Handle */
typedef struct {
    SPI_HandleTypeDef *hspi;        // SPI handle
    GPIO_TypeDef *CS_Port;          // Chip select port
    uint16_t CS_Pin;                // Chip select pin
    GPIO_TypeDef *RESET_Port;       // Reset port
    uint16_t RESET_Pin;             // Reset pin
    GPIO_TypeDef *FAULT_Port;       // Fault port
    uint16_t Fault_Pin;             // Fault pin
    LTC6820_ChannelConfig_t channels[LTC6820_MAX_CHANNELS]; // Channel configurations
    LTC6820_ChannelStatus_t channel_status[LTC6820_MAX_CHANNELS]; // Channel status
    bool is_initialized;            // Initialization flag
    uint32_t uptime;                // Device uptime
} LTC6820_Handle_t;

/* Function Prototypes */

/* Initialization Functions */
LTC6820_Status_t LTC6820_Init(LTC6820_Handle_t *hltc, 
                               SPI_HandleTypeDef *hspi,
                               GPIO_TypeDef *cs_port, 
                               uint16_t cs_pin,
                               GPIO_TypeDef *reset_port,
                               uint16_t reset_pin,
                               GPIO_TypeDef *fault_port,
                               uint16_t fault_pin);

LTC6820_Status_t LTC6820_DeInit(LTC6820_Handle_t *hltc);
LTC6820_Status_t LTC6820_Reset(LTC6820_Handle_t *hltc);

/* Channel Management Functions */
LTC6820_Status_t LTC6820_EnableChannel(LTC6820_Handle_t *hltc, uint8_t channel_id);
LTC6820_Status_t LTC6820_DisableChannel(LTC6820_Handle_t *hltc, uint8_t channel_id);
LTC6820_Status_t LTC6820_ConfigureChannel(LTC6820_Handle_t *hltc, uint8_t channel_id, LTC6820_ChannelConfig_t *config);
LTC6820_Status_t LTC6820_GetChannelStatus(LTC6820_Handle_t *hltc, uint8_t channel_id, LTC6820_ChannelStatus_t *status);

/* Communication Functions */
LTC6820_Status_t LTC6820_WriteChannel(LTC6820_Handle_t *hltc, uint8_t channel_id, uint8_t *data, uint16_t length);
LTC6820_Status_t LTC6820_ReadChannel(LTC6820_Handle_t *hltc, uint8_t channel_id, uint8_t *data, uint16_t length);
LTC6820_Status_t LTC6820_TransmitReceiveChannel(LTC6820_Handle_t *hltc, uint8_t channel_id, uint8_t *tx_data, uint8_t *rx_data, uint16_t length);

/* Broadcast Functions */
LTC6820_Status_t LTC6820_BroadcastWrite(LTC6820_Handle_t *hltc, uint8_t *data, uint16_t length);
LTC6820_Status_t LTC6820_BroadcastRead(LTC6820_Handle_t *hltc, uint8_t *data, uint16_t length);

/* Configuration Functions */
LTC6820_Status_t LTC6820_WriteRegister(LTC6820_Handle_t *hltc, uint8_t reg_addr, uint8_t value);
LTC6820_Status_t LTC6820_ReadRegister(LTC6820_Handle_t *hltc, uint8_t reg_addr, uint8_t *value);
LTC6820_Status_t LTC6820_WriteConfiguration(LTC6820_Handle_t *hltc, uint8_t *config_data, uint16_t length);
LTC6820_Status_t LTC6820_ReadConfiguration(LTC6820_Handle_t *hltc, uint8_t *config_data, uint16_t length);

/* Status and Monitoring Functions */
LTC6820_Status_t LTC6820_GetDeviceStatus(LTC6820_Handle_t *hltc, uint8_t *status);
LTC6820_Status_t LTC6820_GetFaultStatus(LTC6820_Handle_t *hltc, uint8_t *fault_flags);
LTC6820_Status_t LTC6820_ClearFaults(LTC6820_Handle_t *hltc);
bool LTC6820_IsChannelReady(LTC6820_Handle_t *hltc, uint8_t channel_id);
bool LTC6820_HasFault(LTC6820_Handle_t *hltc);

/* Utility Functions */
void LTC6820_DelayUs(uint32_t microseconds);
void LTC6820_CS_Low(LTC6820_Handle_t *hltc);
void LTC6820_CS_High(LTC6820_Handle_t *hltc);
void LTC6820_Reset_Pin_Low(LTC6820_Handle_t *hltc);
void LTC6820_Reset_Pin_High(LTC6820_Handle_t *hltc);
bool LTC6820_IsFaultPinActive(LTC6820_Handle_t *hltc);

/* Error Handling Functions */
void LTC6820_HandleCommunicationError(LTC6820_Handle_t *hltc, uint8_t channel_id);
void LTC6820_HandleFault(LTC6820_Handle_t *hltc);
LTC6820_Status_t LTC6820_RetryCommunication(LTC6820_Handle_t *hltc, uint8_t channel_id, uint8_t *data, uint16_t length);

/* Diagnostic Functions */
void LTC6820_PrintChannelStatus(LTC6820_Handle_t *hltc, uint8_t channel_id);
void LTC6820_PrintDeviceStatus(LTC6820_Handle_t *hltc);
uint32_t LTC6820_GetChannelErrorCount(LTC6820_Handle_t *hltc, uint8_t channel_id);
uint32_t LTC6820_GetChannelTimeoutCount(LTC6820_Handle_t *hltc, uint8_t channel_id);

#ifdef __cplusplus
}
#endif

#endif /* LTC6820_MASTER_H */