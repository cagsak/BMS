/**
 * @file LTC6811_Slave.h
 * @brief Enhanced LTC6811 Slave BMS Driver with NTC Temperature Support
 * @version 1.0
 * @date 2024
 * 
 * This driver extends the basic LTC6811 functionality to include:
 * - 5 NTC temperature sensors
 * - Slave BMS communication protocol
 * - Cell balancing control
 * - Enhanced error handling
 * - Temperature monitoring and conversion
 */

#ifndef LTC6811_SLAVE_H
#define LTC6811_SLAVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "LTC6811.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* Slave BMS Configuration */
#define LTC6811_SLAVE_MAX_CELLS         12      // Maximum cells per slave
#define LTC6811_SLAVE_MAX_TEMP_SENSORS  5       // NTC temperature sensors
#define LTC6811_SLAVE_MAX_BALANCE_CELLS 12      // Maximum cells for balancing
#define LTC6811_SLAVE_COMM_TIMEOUT      1000    // Communication timeout in ms

/* NTC Temperature Sensor Configuration */
typedef struct {
    uint8_t sensor_id;              // Sensor ID (0-4)
    uint16_t beta_value;            // Beta value for NTC calculation
    uint16_t reference_resistance;  // Reference resistance in ohms (25°C)
    uint16_t reference_voltage;     // Reference voltage in millivolts
    uint16_t series_resistance;     // Series resistance in ohms
    int16_t offset_celsius;         // Temperature offset in Celsius
    bool is_enabled;                // Sensor enabled
    uint16_t min_temperature;       // Minimum temperature in Celsius
    uint16_t max_temperature;       // Maximum temperature in Celsius
} LTC6811_NTCConfig_t;

/* NTC Temperature Reading */
typedef struct {
    uint8_t sensor_id;              // Sensor ID
    int16_t temperature_celsius;    // Temperature in Celsius
    uint16_t resistance_ohms;       // Raw resistance in ohms
    uint16_t voltage_mv;            // Voltage reading in millivolts
    bool sensor_fault;              // Sensor fault flag
    bool out_of_range;              // Temperature out of range
    uint32_t last_update;           // Last update timestamp
    uint32_t error_count;           // Error count
} LTC6811_NTCReading_t;

/* Cell Balancing Information */
typedef struct {
    uint8_t cell_id;                // Cell ID (0-11)
    bool balance_enabled;            // Balancing enabled for this cell
    uint32_t balance_start_time;     // When balancing started
    uint32_t balance_duration;       // Total balancing duration
    uint16_t voltage_before;         // Voltage before balancing
    uint16_t voltage_after;          // Voltage after balancing
    uint8_t balance_count;           // Number of balancing cycles
} LTC6811_CellBalance_t;

/* Slave BMS Status */
typedef struct {
    uint8_t slave_id;                               // Unique slave ID
    bool is_active;                                  // Slave is active
    bool communication_error;                        // Communication error
    bool temperature_error;                          // Temperature sensor error
    bool voltage_error;                              // Voltage measurement error
    bool balance_error;                              // Cell balancing error
    uint32_t last_communication;                     // Last communication timestamp
    uint32_t uptime;                                 // Slave uptime in seconds
    uint32_t error_count;                            // Total error count
    uint32_t restart_count;                          // Number of restarts
} LTC6811_SlaveStatus_t;

/* Slave BMS Configuration */
typedef struct {
    uint8_t slave_id;                               // Slave ID
    uint8_t cell_count;                             // Number of cells
    uint8_t temp_sensor_count;                       // Number of temperature sensors
    uint16_t undervoltage_threshold;                 // Undervoltage threshold in mV
    uint16_t overvoltage_threshold;                  // Overvoltage threshold in mV
    int16_t temperature_low_threshold;               // Low temperature threshold
    int16_t temperature_high_threshold;              // High temperature threshold
    uint16_t balance_voltage_threshold;              // Voltage threshold for balancing
    uint16_t balance_voltage_delta;                  // Voltage difference for balancing
    bool auto_balancing_enabled;                     // Enable automatic balancing
    bool emergency_shutdown_enabled;                 // Enable emergency shutdown
    uint16_t update_interval;                        // Update interval in ms
    LTC6811_NTCConfig_t ntc_config[LTC6811_SLAVE_MAX_TEMP_SENSORS]; // NTC configurations
} LTC6811_SlaveConfig_t;

/* Enhanced LTC6811 Slave Handle */
typedef struct {
    LTC6811_Handle_t ltc6811;                       // Base LTC6811 handle
    LTC6811_SlaveConfig_t config;                   // Slave configuration
    LTC6811_SlaveStatus_t status;                   // Slave status
    LTC6811_NTCReading_t temperatures[LTC6811_SLAVE_MAX_TEMP_SENSORS]; // Temperature readings
    LTC6811_CellBalance_t cell_balance[LTC6811_SLAVE_MAX_CELLS]; // Cell balancing info
    uint8_t balance_mask;                            // Current balance mask
    bool balancing_active;                           // Balancing is active
    uint32_t last_balance_check;                    // Last balance check time
    uint32_t last_temperature_update;                // Last temperature update
    uint32_t last_voltage_update;                    // Last voltage update
} LTC6811_SlaveHandle_t;

/* Function Prototypes */

/* Initialization Functions */
LTC6811_Status_t LTC6811_Slave_Init(LTC6811_SlaveHandle_t *hslave, 
                                     SPI_HandleTypeDef *hspi,
                                     GPIO_TypeDef *cs_port, 
                                     uint16_t cs_pin,
                                     uint8_t slave_id);

LTC6811_Status_t LTC6811_Slave_DeInit(LTC6811_SlaveHandle_t *hslave);
LTC6811_Status_t LTC6811_Slave_Reset(LTC6811_SlaveHandle_t *hslave);

/* Configuration Functions */
LTC6811_Status_t LTC6811_Slave_SetConfiguration(LTC6811_SlaveHandle_t *hslave, 
                                                 LTC6811_SlaveConfig_t *config);
LTC6811_Status_t LTC6811_Slave_GetConfiguration(LTC6811_SlaveHandle_t *hslave, 
                                                 LTC6811_SlaveConfig_t *config);
LTC6811_Status_t LTC6811_Slave_SetNTCConfiguration(LTC6811_SlaveHandle_t *hslave, 
                                                    uint8_t sensor_id, 
                                                    LTC6811_NTCConfig_t *ntc_config);
LTC6811_Status_t LTC6811_Slave_SetVoltageThresholds(LTC6811_SlaveHandle_t *hslave, 
                                                     uint16_t undervoltage, 
                                                     uint16_t overvoltage);
LTC6811_Status_t LTC6811_Slave_SetTemperatureThresholds(LTC6811_SlaveHandle_t *hslave, 
                                                         int16_t low_temp, 
                                                         int16_t high_temp);

/* Temperature Monitoring Functions */
LTC6811_Status_t LTC6811_Slave_ReadAllTemperatures(LTC6811_SlaveHandle_t *hslave);
LTC6811_Status_t LTC6811_Slave_ReadTemperature(LTC6811_SlaveHandle_t *hslave, uint8_t sensor_id);
LTC6811_Status_t LTC6811_Slave_StartTemperatureConversion(LTC6811_SlaveHandle_t *hslave);
int16_t LTC6811_Slave_GetTemperature(LTC6811_SlaveHandle_t *hslave, uint8_t sensor_id);
LTC6811_NTCReading_t* LTC6811_Slave_GetTemperatureReading(LTC6811_SlaveHandle_t *hslave, uint8_t sensor_id);

/* Enhanced Cell Monitoring Functions */
LTC6811_Status_t LTC6811_Slave_ReadAllCellVoltages(LTC6811_SlaveHandle_t *hslave);
LTC6811_Status_t LTC6811_Slave_ReadCellVoltage(LTC6811_SlaveHandle_t *hslave, uint8_t cell_id);
uint16_t LTC6811_Slave_GetCellVoltage(LTC6811_SlaveHandle_t *hslave, uint8_t cell_id);
BMS_CellStatus_t LTC6811_Slave_GetCellStatus(LTC6811_SlaveHandle_t *hslave, uint8_t cell_id);

/* Cell Balancing Functions */
LTC6811_Status_t LTC6811_Slave_StartCellBalancing(LTC6811_SlaveHandle_t *hslave, uint8_t cell_mask);
LTC6811_Status_t LTC6811_Slave_StopCellBalancing(LTC6811_SlaveHandle_t *hslave);
LTC6811_Status_t LTC6811_Slave_UpdateCellBalancing(LTC6811_SlaveHandle_t *hslave);
bool LTC6811_Slave_IsCellBalancing(LTC6811_SlaveHandle_t *hslave, uint8_t cell_id);
uint8_t LTC6811_Slave_GetBalanceMask(LTC6811_SlaveHandle_t *hslave);

/* Status and Monitoring Functions */
LTC6811_Status_t LTC6811_Slave_UpdateStatus(LTC6811_SlaveHandle_t *hslave);
LTC6811_SlaveStatus_t* LTC6811_Slave_GetStatus(LTC6811_SlaveHandle_t *hslave);
bool LTC6811_Slave_IsHealthy(LTC6811_SlaveHandle_t *hslave);
bool LTC6811_Slave_HasErrors(LTC6811_SlaveHandle_t *hslave);
uint32_t LTC6811_Slave_GetErrorCount(LTC6811_SlaveHandle_t *hslave);

/* System Management Functions */
LTC6811_Status_t LTC6811_Slave_Sleep(LTC6811_SlaveHandle_t *hslave);
LTC6811_Status_t LTC6811_Slave_WakeUp(LTC6811_SlaveHandle_t *hslave);
LTC6811_Status_t LTC6811_Slave_EmergencyShutdown(LTC6811_SlaveHandle_t *hslave);
LTC6811_Status_t LTC6811_Slave_ResetErrors(LTC6811_SlaveHandle_t *hslave);

/* Data Access Functions */
uint8_t LTC6811_Slave_GetCellCount(LTC6811_SlaveHandle_t *hslave);
uint8_t LTC6811_Slave_GetTemperatureSensorCount(LTC6811_SlaveHandle_t *hslave);
uint16_t LTC6811_Slave_GetTotalVoltage(LTC6811_SlaveHandle_t *hslave);
int16_t LTC6811_Slave_GetAverageTemperature(LTC6811_SlaveHandle_t *hslave);
int16_t LTC6811_Slave_GetMinTemperature(LTC6811_SlaveHandle_t *hslave);
int16_t LTC6811_Slave_GetMaxTemperature(LTC6811_SlaveHandle_t *hslave);

/* Temperature Conversion Functions */
int16_t LTC6811_Slave_ConvertNTCToTemperature(uint16_t voltage_mv, LTC6811_NTCConfig_t *config);
uint16_t LTC6811_Slave_ConvertTemperatureToNTC(int16_t temperature_celsius, LTC6811_NTCConfig_t *config);
uint16_t LTC6811_Slave_CalculateNTCResistance(uint16_t voltage_mv, LTC6811_NTCConfig_t *config);

/* Error Handling Functions */
void LTC6811_Slave_HandleTemperatureError(LTC6811_SlaveHandle_t *hslave, uint8_t sensor_id);
void LTC6811_Slave_HandleVoltageError(LTC6811_SlaveHandle_t *hslave, uint8_t cell_id);
void LTC6811_Slave_HandleCommunicationError(LTC6811_SlaveHandle_t *hslave);
void LTC6811_Slave_LogError(LTC6811_SlaveHandle_t *hslave, uint8_t error_code, uint8_t additional_data);

/* Diagnostic Functions */
void LTC6811_Slave_PrintStatus(LTC6811_SlaveHandle_t *hslave);
void LTC6811_Slave_PrintTemperatureReadings(LTC6811_SlaveHandle_t *hslave);
void LTC6811_Slave_PrintCellVoltages(LTC6811_SlaveHandle_t *hslave);
void LTC6811_Slave_PrintBalancingStatus(LTC6811_SlaveHandle_t *hslave);
void LTC6811_Slave_GenerateDiagnosticReport(LTC6811_SlaveHandle_t *hslave, char *buffer, uint16_t buffer_size);

/* Utility Functions */
uint32_t LTC6811_Slave_GetUptime(LTC6811_SlaveHandle_t *hslave);
bool LTC6811_Slave_IsTimeToUpdate(LTC6811_SlaveHandle_t *hslave);
void LTC6811_Slave_UpdateTimestamps(LTC6811_SlaveHandle_t *hslave);
uint16_t LTC6811_Slave_CalculateChecksum(uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* LTC6811_SLAVE_H */