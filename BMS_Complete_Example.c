/**
 * @file BMS_Complete_Example.c
 * @brief Complete BMS Master-Slave System Example
 * @version 1.0
 * @date 2024
 * 
 * This example demonstrates the complete BMS system including:
 * - Master BMS initialization and management
 * - Slave BMS node communication
 * - Cell voltage monitoring
 * - Temperature monitoring with 5 NTC sensors per slave
 * - Cell balancing
 * - System status monitoring
 * - Error handling and reporting
 */

#include "BMS_Master.h"
#include "LTC6820_Master.h"
#include "LTC6811_Slave.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>

/* External handles - define these in your main.c */
extern SPI_HandleTypeDef hspi1;  // SPI handle for LTC6820
extern GPIO_TypeDef *GPIOA;      // GPIO port

/* System configuration */
#define BMS_EXAMPLE_SLAVE_COUNT     4       // Number of slave BMS nodes
#define BMS_EXAMPLE_CELLS_PER_SLAVE 12      // Cells per slave
#define BMS_EXAMPLE_TEMP_SENSORS    5       // Temperature sensors per slave
#define BMS_EXAMPLE_UPDATE_INTERVAL 100     // Update interval in ms
#define BMS_EXAMPLE_BALANCE_CHECK   5000    // Balance check interval in ms

/* Function prototypes */
void BMS_Example_InitializeSystem(void);
void BMS_Example_MainLoop(void);
void BMS_Example_ReadAllSlaves(void);
void BMS_Example_ProcessSlaveData(void);
void BMS_Example_CheckCellBalancing(void);
void BMS_Example_GenerateSystemReport(void);
void BMS_Example_HandleAlarms(void);
void BMS_Example_PrintSlaveStatus(uint8_t slave_id);
void BMS_Example_PrintCellVoltages(uint8_t slave_id);
void BMS_Example_PrintTemperatures(uint8_t slave_id);
void BMS_Example_PrintSystemSummary(void);

/**
 * @brief Main BMS example function
 */
void BMS_Example_Main(void)
{
    printf("========================================\r\n");
    printf("Complete BMS Master-Slave System Example\r\n");
    printf("========================================\r\n");
    printf("Architecture: Master BMS + %d Slave BMS Nodes\r\n", BMS_EXAMPLE_SLAVE_COUNT);
    printf("Cells per slave: %d\r\n", BMS_EXAMPLE_CELLS_PER_SLAVE);
    printf("Temperature sensors per slave: %d\r\n", BMS_EXAMPLE_TEMP_SENSORS);
    printf("Total cells: %d\r\n", BMS_EXAMPLE_SLAVE_COUNT * BMS_EXAMPLE_CELLS_PER_SLAVE);
    printf("Total temperature sensors: %d\r\n", BMS_EXAMPLE_SLAVE_COUNT * BMS_EXAMPLE_TEMP_SENSORS);
    printf("========================================\r\n");
    
    /* Initialize the complete BMS system */
    BMS_Example_InitializeSystem();
    
    /* Main system loop */
    BMS_Example_MainLoop();
}

/**
 * @brief Initialize the complete BMS system
 */
void BMS_Example_InitializeSystem(void)
{
    printf("Initializing BMS System...\r\n");
    
    /* Initialize Master BMS */
    BMS_ResponseStatus_t status = BMS_Master_Init();
    if (status != BMS_RESPONSE_OK) {
        printf("ERROR: Master BMS initialization failed: %d\r\n", status);
        return;
    }
    printf("✓ Master BMS initialized\r\n");
    
    /* Configure system parameters */
    BMS_Configuration_t config;
    memset(&config, 0, sizeof(BMS_Configuration_t));
    
    /* Set voltage thresholds for Li-ion cells */
    config.cell_undervoltage_threshold = 3000;  // 3.0V
    config.cell_overvoltage_threshold = 4200;   // 4.2V
    config.temperature_low_threshold = -20;     // -20°C
    config.temperature_high_threshold = 60;     // 60°C
    config.balance_voltage_threshold = 3500;    // 3.5V
    config.balance_voltage_delta = 50;          // 50mV
    config.update_interval = BMS_EXAMPLE_UPDATE_INTERVAL;
    config.auto_balancing_enabled = true;
    config.emergency_shutdown_enabled = true;
    
    /* Configure NTC temperature sensors */
    for (int i = 0; i < BMS_MAX_TEMP_SENSORS; i++) {
        config.temp_config[i].beta_value = 3950;           // Standard NTC beta value
        config.temp_config[i].reference_resistance = 10000; // 10kΩ at 25°C
        config.temp_config[i].reference_voltage = 3300;     // 3.3V reference
        config.temp_config[i].series_resistance = 10000;    // 10kΩ series resistor
        config.temp_config[i].offset_celsius = 0;           // No offset
        config.temp_config[i].is_enabled = true;
        config.temp_config[i].min_temperature = -40;
        config.temp_config[i].max_temperature = 125;
    }
    
    /* Apply configuration */
    status = BMS_Master_SetConfiguration(&config);
    if (status != BMS_RESPONSE_OK) {
        printf("ERROR: Failed to set BMS configuration: %d\r\n", status);
        return;
    }
    printf("✓ BMS configuration applied\r\n");
    
    /* Activate slave nodes (simulate discovery) */
    for (int i = 0; i < BMS_EXAMPLE_SLAVE_COUNT; i++) {
        BMS_SlaveNode_t *slave = BMS_Master_GetSlaveInfo(i);
        if (slave != NULL) {
            slave->is_active = true;
            slave->cell_count = BMS_EXAMPLE_CELLS_PER_SLAVE;
            slave->last_communication = HAL_GetTick();
            printf("✓ Slave %d activated\r\n", i);
        }
    }
    
    printf("✓ BMS System initialization complete\r\n");
    printf("Active slaves: %d\r\n", BMS_GetActiveSlaveCount());
    printf("Total cells: %d\r\n", BMS_GetTotalCellCount());
}

/**
 * @brief Main system loop
 */
void BMS_Example_MainLoop(void)
{
    static uint32_t last_balance_check = 0;
    static uint32_t last_report = 0;
    uint32_t current_time = HAL_GetTick();
    
    printf("Starting BMS system main loop...\r\n");
    
    while (1) {
        current_time = HAL_GetTick();
        
        /* Update BMS master system */
        BMS_ResponseStatus_t status = BMS_Master_Update();
        if (status != BMS_RESPONSE_OK) {
            printf("WARNING: BMS update failed: %d\r\n", status);
        }
        
        /* Read all slave data */
        BMS_Example_ReadAllSlaves();
        
        /* Process slave data */
        BMS_Example_ProcessSlaveData();
        
        /* Check cell balancing periodically */
        if (current_time - last_balance_check >= BMS_EXAMPLE_BALANCE_CHECK) {
            BMS_Example_CheckCellBalancing();
            last_balance_check = current_time;
        }
        
        /* Generate system report periodically */
        if (current_time - last_report >= 10000) { // Every 10 seconds
            BMS_Example_GenerateSystemReport();
            last_report = current_time;
        }
        
        /* Handle any alarms */
        BMS_Example_HandleAlarms();
        
        /* Check system health */
        if (!BMS_IsSystemHealthy()) {
            printf("WARNING: System health check failed\r\n");
        }
        
        /* Wait for next update cycle */
        HAL_Delay(BMS_EXAMPLE_UPDATE_INTERVAL);
    }
}

/**
 * @brief Read data from all slave BMS nodes
 */
void BMS_Example_ReadAllSlaves(void)
{
    for (int i = 0; i < BMS_EXAMPLE_SLAVE_COUNT; i++) {
        BMS_SlaveNode_t *slave = BMS_Master_GetSlaveInfo(i);
        if (slave != NULL && slave->is_active) {
            BMS_ResponseStatus_t status = BMS_Master_ReadSlave(i);
            if (status != BMS_RESPONSE_OK) {
                printf("WARNING: Failed to read slave %d: %d\r\n", i, status);
                slave->communication_error = true;
            } else {
                slave->communication_error = false;
            }
        }
    }
}

/**
 * @brief Process data from all slave nodes
 */
void BMS_Example_ProcessSlaveData(void)
{
    for (int i = 0; i < BMS_EXAMPLE_SLAVE_COUNT; i++) {
        BMS_SlaveNode_t *slave = BMS_Master_GetSlaveInfo(i);
        if (slave != NULL && slave->is_active) {
            /* Process cell voltage data */
            for (int j = 0; j < BMS_EXAMPLE_CELLS_PER_SLAVE; j++) {
                BMS_CellInfo_t *cell = &slave->cells[j];
                
                /* Check voltage thresholds */
                if (cell->voltage_mv < 3000) {
                    cell->status = CELL_STATUS_UNDERVOLTAGE;
                } else if (cell->voltage_mv > 4200) {
                    cell->status = CELL_STATUS_OVERVOLTAGE;
                } else {
                    cell->status = CELL_STATUS_NORMAL;
                }
                
                /* Update cell timestamp */
                cell->last_update = HAL_GetTick();
            }
            
            /* Process temperature data */
            for (int j = 0; j < BMS_EXAMPLE_TEMP_SENSORS; j++) {
                BMS_TempInfo_t *temp = &slave->temperatures[j];
                
                /* Check temperature thresholds */
                if (temp->temperature_celsius < -20 || temp->temperature_celsius > 60) {
                    temp->sensor_fault = true;
                } else {
                    temp->sensor_fault = false;
                }
                
                /* Update temperature timestamp */
                temp->last_update = HAL_GetTick();
            }
        }
    }
}

/**
 * @brief Check and manage cell balancing
 */
void BMS_Example_CheckCellBalancing(void)
{
    printf("Checking cell balancing...\r\n");
    
    for (int i = 0; i < BMS_EXAMPLE_SLAVE_COUNT; i++) {
        BMS_SlaveNode_t *slave = BMS_Master_GetSlaveInfo(i);
        if (slave != NULL && slave->is_active) {
            uint8_t balance_mask = 0;
            bool balancing_needed = false;
            
            /* Check if balancing is needed */
            for (int j = 0; j < BMS_EXAMPLE_CELLS_PER_SLAVE; j++) {
                BMS_CellInfo_t *cell = &slave->cells[j];
                
                if (cell->voltage_mv > 3500) { // Above balance threshold
                    balance_mask |= (1 << j);
                    balancing_needed = true;
                }
            }
            
            /* Start or stop balancing */
            if (balancing_needed) {
                BMS_ResponseStatus_t status = BMS_Master_StartCellBalancing(i, balance_mask);
                if (status == BMS_RESPONSE_OK) {
                    printf("Started balancing for slave %d, mask: 0x%02X\r\n", i, balance_mask);
                }
            } else {
                BMS_ResponseStatus_t status = BMS_Master_StopCellBalancing(i);
                if (status == BMS_RESPONSE_OK) {
                    printf("Stopped balancing for slave %d\r\n", i);
                }
            }
        }
    }
}

/**
 * @brief Generate comprehensive system report
 */
void BMS_Example_GenerateSystemReport(void)
{
    printf("\r\n========================================\r\n");
    printf("BMS System Status Report\r\n");
    printf("========================================\r\n");
    
    /* System overview */
    BMS_SystemStatus_t system_status = BMS_Master_GetSystemStatus();
    printf("System Status: %d\r\n", system_status);
    printf("Active Slaves: %d\r\n", BMS_GetActiveSlaveCount());
    printf("Total Cells: %d\r\n", BMS_GetTotalCellCount());
    printf("System Voltage: %d mV\r\n", BMS_Master_GetTotalSystemVoltage());
    printf("System Temperature: %d°C\r\n", BMS_Master_GetSystemTemperature());
    printf("System Uptime: %lu seconds\r\n", HAL_GetTick() / 1000);
    
    /* Slave details */
    for (int i = 0; i < BMS_EXAMPLE_SLAVE_COUNT; i++) {
        BMS_SlaveNode_t *slave = BMS_Master_GetSlaveInfo(i);
        if (slave != NULL && slave->is_active) {
            printf("\r\n--- Slave %d ---\r\n", i);
            BMS_Example_PrintSlaveStatus(i);
        }
    }
    
    printf("========================================\r\n\r\n");
}

/**
 * @brief Handle system alarms and critical conditions
 */
void BMS_Example_HandleAlarms(void)
{
    BMS_SystemStatus_t system_status = BMS_Master_GetSystemStatus();
    
    switch (system_status) {
        case BMS_SYSTEM_OK:
            // No action needed
            break;
            
        case BMS_SYSTEM_WARNING:
            printf("WARNING: System has warnings\r\n");
            break;
            
        case BMS_SYSTEM_ERROR:
            printf("ERROR: System has errors\r\n");
            break;
            
        case BMS_SYSTEM_CRITICAL:
            printf("CRITICAL: System is critical\r\n");
            break;
            
        case BMS_SYSTEM_EMERGENCY:
            printf("EMERGENCY: System shutdown\r\n");
            break;
            
        default:
            printf("UNKNOWN: Unknown system status\r\n");
            break;
    }
    
    /* Check individual slave alarms */
    for (int i = 0; i < BMS_EXAMPLE_SLAVE_COUNT; i++) {
        BMS_SlaveNode_t *slave = BMS_Master_GetSlaveInfo(i);
        if (slave != NULL && slave->is_active) {
            /* Check for communication errors */
            if (slave->communication_error) {
                printf("ALARM: Slave %d communication error\r\n", i);
            }
            
            /* Check for voltage violations */
            for (int j = 0; j < BMS_EXAMPLE_CELLS_PER_SLAVE; j++) {
                BMS_CellInfo_t *cell = &slave->cells[j];
                if (cell->status == CELL_STATUS_OVERVOLTAGE) {
                    printf("ALARM: Slave %d, Cell %d overvoltage: %d mV\r\n", i, j, cell->voltage_mv);
                } else if (cell->status == CELL_STATUS_UNDERVOLTAGE) {
                    printf("ALARM: Slave %d, Cell %d undervoltage: %d mV\r\n", i, j, cell->voltage_mv);
                }
            }
            
            /* Check for temperature violations */
            for (int j = 0; j < BMS_EXAMPLE_TEMP_SENSORS; j++) {
                BMS_TempInfo_t *temp = &slave->temperatures[j];
                if (temp->sensor_fault) {
                    printf("ALARM: Slave %d, Temp %d fault: %d°C\r\n", i, j, temp->temperature_celsius);
                }
            }
        }
    }
}

/**
 * @brief Print status for a specific slave
 */
void BMS_Example_PrintSlaveStatus(uint8_t slave_id)
{
    BMS_SlaveNode_t *slave = BMS_Master_GetSlaveInfo(slave_id);
    if (slave == NULL) {
        printf("Slave %d not found\r\n", slave_id);
        return;
    }
    
    printf("Status: %s\r\n", slave->is_active ? "Active" : "Inactive");
    printf("Communication: %s\r\n", slave->communication_error ? "Error" : "OK");
    printf("Cell Count: %d\r\n", slave->cell_count);
    printf("Last Communication: %lu ms ago\r\n", HAL_GetTick() - slave->last_communication);
    printf("Uptime: %lu seconds\r\n", slave->uptime);
    printf("Error Flags: 0x%02X\r\n", slave->error_flags);
    
    /* Print cell voltages */
    BMS_Example_PrintCellVoltages(slave_id);
    
    /* Print temperatures */
    BMS_Example_PrintTemperatures(slave_id);
}

/**
 * @brief Print cell voltages for a specific slave
 */
void BMS_Example_PrintCellVoltages(uint8_t slave_id)
{
    BMS_SlaveNode_t *slave = BMS_Master_GetSlaveInfo(slave_id);
    if (slave == NULL) {
        return;
    }
    
    printf("Cell Voltages:\r\n");
    for (int i = 0; i < BMS_EXAMPLE_CELLS_PER_SLAVE; i++) {
        BMS_CellInfo_t *cell = &slave->cells[i];
        printf("  Cell %2d: %4d mV [%s]\r\n", i, cell->voltage_mv, 
               cell->status == CELL_STATUS_NORMAL ? "OK" :
               cell->status == CELL_STATUS_OVERVOLTAGE ? "OV" :
               cell->status == CELL_STATUS_UNDERVOLTAGE ? "UV" : "??");
    }
}

/**
 * @brief Print temperatures for a specific slave
 */
void BMS_Example_PrintTemperatures(uint8_t slave_id)
{
    BMS_SlaveNode_t *slave = BMS_Master_GetSlaveInfo(slave_id);
    if (slave == NULL) {
        return;
    }
    
    printf("Temperatures:\r\n");
    for (int i = 0; i < BMS_EXAMPLE_TEMP_SENSORS; i++) {
        BMS_TempInfo_t *temp = &slave->temperatures[i];
        printf("  Temp %d: %3d°C [%s]\r\n", i, temp->temperature_celsius,
               temp->sensor_fault ? "FAULT" : "OK");
    }
}

/**
 * @brief Print system summary
 */
void BMS_Example_PrintSystemSummary(void)
{
    printf("\r\nBMS System Summary:\r\n");
    printf("==================\r\n");
    printf("Status: %d\r\n", BMS_Master_GetSystemStatus());
    printf("Active Slaves: %d\r\n", BMS_GetActiveSlaveCount());
    printf("Total Voltage: %d mV\r\n", BMS_Master_GetTotalSystemVoltage());
    printf("Average Temp: %d°C\r\n", BMS_Master_GetSystemTemperature());
    printf("System Healthy: %s\r\n", BMS_IsSystemHealthy() ? "Yes" : "No");
    printf("==================\r\n\r\n");
}

/**
 * @brief Example of emergency procedures
 */
void BMS_Example_EmergencyProcedures(void)
{
    printf("EMERGENCY: Initiating emergency procedures\r\n");
    
    /* Emergency shutdown */
    BMS_ResponseStatus_t status = BMS_Master_EmergencyShutdown();
    if (status == BMS_RESPONSE_OK) {
        printf("Emergency shutdown completed\r\n");
    } else {
        printf("Emergency shutdown failed: %d\r\n", status);
    }
    
    /* Wait for system to stabilize */
    HAL_Delay(1000);
    
    /* Reset system */
    status = BMS_Master_ResetSystem();
    if (status == BMS_RESPONSE_OK) {
        printf("System reset completed\r\n");
    } else {
        printf("System reset failed: %d\r\n", status);
    }
}

/**
 * @brief Example of configuration management
 */
void BMS_Example_ConfigurationManagement(void)
{
    printf("Managing BMS configuration...\r\n");
    
    /* Read current configuration */
    BMS_Configuration_t current_config;
    BMS_ResponseStatus_t status = BMS_Master_GetConfiguration(&current_config);
    if (status == BMS_RESPONSE_OK) {
        printf("Current undervoltage threshold: %d mV\r\n", current_config.cell_undervoltage_threshold);
        printf("Current overvoltage threshold: %d mV\r\n", current_config.cell_overvoltage_threshold);
        printf("Current temperature thresholds: %d°C to %d°C\r\n", 
               current_config.temperature_low_threshold, current_config.temperature_high_threshold);
    }
    
    /* Modify configuration */
    status = BMS_Master_SetCellThresholds(2800, 4300); // More conservative thresholds
    if (status == BMS_RESPONSE_OK) {
        printf("Updated cell voltage thresholds\r\n");
    }
    
    status = BMS_Master_SetTemperatureThresholds(-30, 70); // Wider temperature range
    if (status == BMS_RESPONSE_OK) {
        printf("Updated temperature thresholds\r\n");
    }
}