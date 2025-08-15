/**
 * @file BMS_Architecture.h
 * @brief Master-Slave BMS Architecture with LTC6820 Master and LTC6811 Slaves
 * @version 1.0
 * @date 2024
 * 
 * This file defines the complete BMS architecture including:
 * - Master BMS with LTC6820 isolated SPI interface
 * - Slave BMS nodes with LTC6811 cell monitoring
 * - 5 NTC temperature sensors per slave
 * - Communication protocol between master and slaves
 */

#ifndef BMS_ARCHITECTURE_H
#define BMS_ARCHITECTURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* System Configuration */
#define BMS_MAX_SLAVES              8       // Maximum number of slave BMS nodes
#define BMS_MAX_CELLS_PER_SLAVE     12      // Maximum cells per slave (LTC6811 limit)
#define BMS_MAX_TEMP_SENSORS        5       // NTC temperature sensors per slave
#define BMS_MAX_TOTAL_CELLS         (BMS_MAX_SLAVES * BMS_MAX_CELLS_PER_SLAVE)
#define BMS_MAX_TOTAL_TEMP_SENSORS  (BMS_MAX_SLAVES * BMS_MAX_TEMP_SENSORS)

/* Communication Protocol */
#define BMS_CMD_HEADER              0xAA    // Command header byte
#define BMS_CMD_FOOTER              0x55    // Command footer byte
#define BMS_MAX_PACKET_SIZE         64      // Maximum packet size
#define BMS_TIMEOUT_MS              1000    // Communication timeout in ms

/* Command Types */
typedef enum {
    BMS_CMD_NONE = 0x00,
    BMS_CMD_READ_CELL_VOLTAGES = 0x01,
    BMS_CMD_READ_TEMPERATURES = 0x02,
    BMS_CMD_READ_STATUS = 0x03,
    BMS_CMD_WRITE_CONFIG = 0x04,
    BMS_CMD_READ_CONFIG = 0x05,
    BMS_CMD_START_CONVERSION = 0x06,
    BMS_CMD_SLEEP = 0x07,
    BMS_CMD_WAKE = 0x08,
    BMS_CMD_EMERGENCY_SHUTDOWN = 0x09,
    BMS_CMD_BALANCE_CELLS = 0x0A,
    BMS_CMD_READ_BALANCE_STATUS = 0x0B,
    BMS_CMD_READ_SYSTEM_STATUS = 0x0C,
    BMS_CMD_RESET = 0x0D,
    BMS_CMD_ACK = 0x0E,
    BMS_CMD_NACK = 0x0F
} BMS_Command_t;

/* Response Status */
typedef enum {
    BMS_RESPONSE_OK = 0x00,
    BMS_RESPONSE_ERROR = 0x01,
    BMS_RESPONSE_TIMEOUT = 0x02,
    BMS_RESPONSE_INVALID_CMD = 0x03,
    BMS_RESPONSE_INVALID_PARAM = 0x04,
    BMS_RESPONSE_BUSY = 0x05
} BMS_ResponseStatus_t;

/* Cell Status */
typedef enum {
    CELL_STATUS_NORMAL = 0x00,
    CELL_STATUS_UNDERVOLTAGE = 0x01,
    CELL_STATUS_OVERVOLTAGE = 0x02,
    CELL_STATUS_OVERTEMPERATURE = 0x03,
    CELL_STATUS_FAULT = 0x04
} BMS_CellStatus_t;

/* System Status */
typedef enum {
    BMS_SYSTEM_OK = 0x00,
    BMS_SYSTEM_WARNING = 0x01,
    BMS_SYSTEM_ERROR = 0x02,
    BMS_SYSTEM_CRITICAL = 0x03,
    BMS_SYSTEM_EMERGENCY = 0x04
} BMS_SystemStatus_t;

/* Temperature Sensor Configuration */
typedef struct {
    uint16_t beta_value;           // Beta value for NTC calculation
    uint16_t reference_resistance; // Reference resistance in ohms
    uint16_t reference_voltage;    // Reference voltage in millivolts
    uint16_t series_resistance;    // Series resistance in ohms
    int16_t offset_celsius;        // Temperature offset in Celsius
} BMS_TempSensorConfig_t;

/* Cell Information */
typedef struct {
    uint16_t voltage_mv;           // Cell voltage in millivolts
    BMS_CellStatus_t status;       // Cell status
    uint8_t balance_enabled;       // Cell balancing enabled
    uint32_t last_update;          // Last update timestamp
} BMS_CellInfo_t;

/* Temperature Information */
typedef struct {
    int16_t temperature_celsius;    // Temperature in Celsius
    uint16_t resistance_ohms;      // Raw resistance in ohms
    uint16_t voltage_mv;           // Voltage reading in millivolts
    bool sensor_fault;             // Sensor fault flag
    uint32_t last_update;          // Last update timestamp
} BMS_TempInfo_t;

/* Slave BMS Node Information */
typedef struct {
    uint8_t slave_id;                              // Unique slave ID
    bool is_active;                                 // Slave is active and responding
    bool communication_error;                       // Communication error flag
    uint32_t last_communication;                   // Last successful communication
    uint8_t cell_count;                            // Number of cells on this slave
    BMS_CellInfo_t cells[BMS_MAX_CELLS_PER_SLAVE]; // Cell information array
    BMS_TempInfo_t temperatures[BMS_MAX_TEMP_SENSORS]; // Temperature information array
    uint16_t slave_voltage;                        // Slave power supply voltage
    uint8_t error_flags;                           // Error flags from slave
    uint32_t uptime;                               // Slave uptime in seconds
} BMS_SlaveNode_t;

/* Master BMS System Information */
typedef struct {
    BMS_SystemStatus_t system_status;              // Overall system status
    uint8_t active_slave_count;                    // Number of active slaves
    uint16_t total_cell_count;                     // Total number of cells
    uint16_t total_temp_sensor_count;              // Total number of temperature sensors
    uint16_t system_voltage;                       // Total system voltage
    uint16_t system_current;                       // System current in mA
    int16_t system_temperature;                    // System temperature
    uint32_t system_uptime;                        // System uptime in seconds
    uint32_t last_system_update;                   // Last system update timestamp
    BMS_SlaveNode_t slaves[BMS_MAX_SLAVES];        // Array of slave nodes
} BMS_MasterSystem_t;

/* Communication Packet Structure */
typedef struct {
    uint8_t header;                                // Packet header (0xAA)
    uint8_t slave_id;                              // Target slave ID (0xFF for broadcast)
    uint8_t command;                               // Command type
    uint8_t data_length;                           // Data length
    uint8_t data[BMS_MAX_PACKET_SIZE - 8];        // Data payload
    uint16_t checksum;                             // Packet checksum
    uint8_t footer;                                // Packet footer (0x55)
} BMS_CommunicationPacket_t;

/* Configuration Structure */
typedef struct {
    uint16_t cell_undervoltage_threshold;          // Undervoltage threshold in mV
    uint16_t cell_overvoltage_threshold;           // Overvoltage threshold in mV
    int16_t temperature_high_threshold;            // High temperature threshold in Celsius
    int16_t temperature_low_threshold;             // Low temperature threshold in Celsius
    uint16_t balance_voltage_threshold;            // Voltage threshold for balancing in mV
    uint16_t balance_voltage_delta;                // Voltage difference for balancing in mV
    uint16_t communication_timeout;                // Communication timeout in ms
    uint16_t update_interval;                      // Update interval in ms
    uint8_t max_retry_count;                       // Maximum retry count for communication
    bool auto_balancing_enabled;                   // Enable automatic cell balancing
    bool emergency_shutdown_enabled;               // Enable emergency shutdown
    BMS_TempSensorConfig_t temp_config[BMS_MAX_TEMP_SENSORS]; // Temperature sensor configs
} BMS_Configuration_t;

/* Function Prototypes */

/* Master BMS Functions */
BMS_ResponseStatus_t BMS_Master_Init(void);
BMS_ResponseStatus_t BMS_Master_DeInit(void);
BMS_ResponseStatus_t BMS_Master_Update(void);
BMS_ResponseStatus_t BMS_Master_ReadAllSlaves(void);
BMS_ResponseStatus_t BMS_Master_ReadSlave(uint8_t slave_id);
BMS_ResponseStatus_t BMS_Master_SendCommand(uint8_t slave_id, BMS_Command_t command, uint8_t *data, uint8_t data_length);
BMS_ResponseStatus_t BMS_Master_BroadcastCommand(BMS_Command_t command, uint8_t *data, uint8_t data_length);

/* Slave Communication Functions */
BMS_ResponseStatus_t BMS_Master_ReadSlaveCellVoltages(uint8_t slave_id);
BMS_ResponseStatus_t BMS_Master_ReadSlaveTemperatures(uint8_t slave_id);
BMS_ResponseStatus_t BMS_Master_ReadSlaveStatus(uint8_t slave_id);
BMS_ResponseStatus_t BMS_Master_WriteSlaveConfig(uint8_t slave_id, BMS_Configuration_t *config);
BMS_ResponseStatus_t BMS_Master_ReadSlaveConfig(uint8_t slave_id, BMS_Configuration_t *config);

/* System Management Functions */
BMS_SystemStatus_t BMS_Master_GetSystemStatus(void);
BMS_ResponseStatus_t BMS_Master_StartCellBalancing(uint8_t slave_id, uint8_t cell_mask);
BMS_ResponseStatus_t BMS_Master_StopCellBalancing(uint8_t slave_id);
BMS_ResponseStatus_t BMS_Master_EmergencyShutdown(void);
BMS_ResponseStatus_t BMS_Master_ResetSystem(void);

/* Data Access Functions */
BMS_CellInfo_t* BMS_Master_GetCellInfo(uint8_t slave_id, uint8_t cell_id);
BMS_TempInfo_t* BMS_Master_GetTemperatureInfo(uint8_t slave_id, uint8_t sensor_id);
BMS_SlaveNode_t* BMS_Master_GetSlaveInfo(uint8_t slave_id);
uint16_t BMS_Master_GetTotalSystemVoltage(void);
int16_t BMS_Master_GetSystemTemperature(void);

/* Configuration Functions */
BMS_ResponseStatus_t BMS_Master_SetConfiguration(BMS_Configuration_t *config);
BMS_ResponseStatus_t BMS_Master_GetConfiguration(BMS_Configuration_t *config);
BMS_ResponseStatus_t BMS_Master_SetCellThresholds(uint16_t undervoltage, uint16_t overvoltage);
BMS_ResponseStatus_t BMS_Master_SetTemperatureThresholds(int16_t low_temp, int16_t high_temp);

/* Utility Functions */
uint16_t BMS_CalculateChecksum(uint8_t *data, uint8_t length);
bool BMS_ValidateChecksum(uint8_t *data, uint8_t length, uint16_t checksum);
uint8_t BMS_GetActiveSlaveCount(void);
uint16_t BMS_GetTotalCellCount(void);
bool BMS_IsSystemHealthy(void);

/* Temperature Conversion Functions */
int16_t BMS_ConvertNTCToTemperature(uint16_t voltage_mv, BMS_TempSensorConfig_t *config);
uint16_t BMS_ConvertTemperatureToNTC(int16_t temperature_celsius, BMS_TempSensorConfig_t *config);

/* Error Handling Functions */
void BMS_Master_HandleCommunicationError(uint8_t slave_id);
void BMS_Master_HandleSlaveError(uint8_t slave_id, uint8_t error_flags);
void BMS_Master_LogError(uint8_t error_code, uint8_t slave_id, uint8_t additional_data);

/* Status Reporting Functions */
void BMS_Master_GenerateStatusReport(char *buffer, uint16_t buffer_size);
void BMS_Master_GenerateAlarmReport(char *buffer, uint16_t buffer_size);
void BMS_Master_GenerateDiagnosticReport(char *buffer, uint16_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* BMS_ARCHITECTURE_H */