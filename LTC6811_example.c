/**
 * @file LTC6811_example.c
 * @brief Example usage of LTC6811 driver for STM32
 * @version 1.0
 * @date 2024
 * 
 * This file demonstrates how to use the LTC6811 driver to monitor
 * battery cells in a battery management system.
 */

#include "LTC6811.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>

/* External SPI and GPIO handles - define these in your main.c or stm32f4xx_it.c */
extern SPI_HandleTypeDef hspi1;  // Your SPI handle
extern GPIO_TypeDef *GPIOA;      // Your GPIO port for chip select

/* LTC6811 handle */
LTC6811_Handle_t hltc6811;

/* Function prototypes */
void LTC6811_Example_Init(void);
void LTC6811_Example_ReadAllCells(void);
void LTC6811_Example_ReadAuxiliary(void);
void LTC6811_Example_ReadStatus(void);
void LTC6811_Example_ConfigureThresholds(void);
void LTC6811_Example_PrintCellVoltages(void);
void LTC6811_Example_PrintAuxiliaryData(void);
void LTC6811_Example_PrintStatus(void);

/**
 * @brief Main example function
 */
void LTC6811_Example_Main(void)
{
    printf("LTC6811 Battery Monitor Example\r\n");
    printf("==============================\r\n");
    
    /* Initialize LTC6811 */
    LTC6811_Example_Init();
    
    /* Configure voltage thresholds */
    LTC6811_Example_ConfigureThresholds();
    
    /* Main loop */
    while (1) {
        /* Read all cell voltages */
        LTC6811_Example_ReadAllCells();
        
        /* Read auxiliary data */
        LTC6811_Example_ReadAuxiliary();
        
        /* Read status */
        LTC6811_Example_ReadStatus();
        
        /* Print all data */
        LTC6811_Example_PrintCellVoltages();
        LTC6811_Example_PrintAuxiliaryData();
        LTC6811_Example_PrintStatus();
        
        /* Check for voltage violations */
        if (LTC6811_IsOvervoltage(&hltc6811)) {
            printf("WARNING: Overvoltage detected!\r\n");
        }
        
        if (LTC6811_IsUndervoltage(&hltc6811)) {
            printf("WARNING: Undervoltage detected!\r\n");
        }
        
        printf("----------------------------------------\r\n");
        
        /* Wait 1 second before next reading */
        HAL_Delay(1000);
    }
}

/**
 * @brief Initialize LTC6811 device
 */
void LTC6811_Example_Init(void)
{
    printf("Initializing LTC6811...\r\n");
    
    /* Initialize LTC6811 with SPI1 and PA4 as chip select */
    LTC6811_Status_t status = LTC6811_Init(&hltc6811, &hspi1, GPIOA, GPIO_PIN_4);
    
    if (status != LTC6811_OK) {
        printf("ERROR: LTC6811 initialization failed with status: %d\r\n", status);
        return;
    }
    
    printf("LTC6811 initialized successfully!\r\n");
    
    /* Read current configuration */
    LTC6811_Config_t config;
    status = LTC6811_ReadConfig(&hltc6811, &config);
    if (status == LTC6811_OK) {
        printf("Configuration read successfully\r\n");
        printf("CFGR0: 0x%02X\r\n", config.CFGR0);
        printf("CFGR1: 0x%02X\r\n", config.CFGR1);
        printf("CFGR2: 0x%02X\r\n", config.CFGR2);
        printf("CFGR3: 0x%02X\r\n", config.CFGR3);
        printf("CFGR4: 0x%02X\r\n", config.CFGR4);
        printf("CFGR5: 0x%02X\r\n", config.CFGR5);
    } else {
        printf("ERROR: Failed to read configuration\r\n");
    }
}

/**
 * @brief Read all cell voltages
 */
void LTC6811_Example_ReadAllCells(void)
{
    /* Start cell voltage conversion */
    LTC6811_Status_t status = LTC6811_StartCellVoltageConversion(&hltc6811, 0);
    if (status != LTC6811_OK) {
        printf("ERROR: Failed to start cell voltage conversion\r\n");
        return;
    }
    
    /* Read cell voltages */
    status = LTC6811_ReadCellVoltages(&hltc6811);
    if (status != LTC6811_OK) {
        printf("ERROR: Failed to read cell voltages\r\n");
        return;
    }
    
    printf("Cell voltages read successfully\r\n");
}

/**
 * @brief Read auxiliary data
 */
void LTC6811_Example_ReadAuxiliary(void)
{
    /* Start auxiliary conversion */
    LTC6811_Status_t status = LTC6811_StartAuxConversion(&hltc6811, 0);
    if (status != LTC6811_OK) {
        printf("ERROR: Failed to start auxiliary conversion\r\n");
        return;
    }
    
    /* Read auxiliary data */
    status = LTC6811_ReadAuxiliary(&hltc6811);
    if (status != LTC6811_OK) {
        printf("ERROR: Failed to read auxiliary data\r\n");
        return;
    }
    
    printf("Auxiliary data read successfully\r\n");
}

/**
 * @brief Read status registers
 */
void LTC6811_Example_ReadStatus(void)
{
    LTC6811_Status_t status = LTC6811_ReadStatus(&hltc6811);
    if (status != LTC6811_OK) {
        printf("ERROR: Failed to read status\r\n");
        return;
    }
    
    printf("Status read successfully\r\n");
}

/**
 * @brief Configure voltage thresholds
 */
void LTC6811_Example_ConfigureThresholds(void)
{
    printf("Configuring voltage thresholds...\r\n");
    
    /* Set undervoltage threshold to 3.0V (3000mV) */
    LTC6811_Status_t status = LTC6811_SetUndervoltageThreshold(&hltc6811, 3000);
    if (status == LTC6811_OK) {
        printf("Undervoltage threshold set to 3.0V\r\n");
    } else {
        printf("ERROR: Failed to set undervoltage threshold\r\n");
    }
    
    /* Set overvoltage threshold to 4.2V (4200mV) */
    status = LTC6811_SetOvervoltageThreshold(&hltc6811, 4200);
    if (status == LTC6811_OK) {
        printf("Overvoltage threshold set to 4.2V\r\n");
    } else {
        printf("ERROR: Failed to set overvoltage threshold\r\n");
    }
    
    /* Enable GPIO channels 1-5 */
    status = LTC6811_SetGPIOConfig(&hltc6811, 0x1F);
    if (status == LTC6811_OK) {
        printf("GPIO channels 1-5 enabled\r\n");
    } else {
        printf("ERROR: Failed to configure GPIO\r\n");
    }
    
    /* Set ADC options for high accuracy */
    status = LTC6811_SetADCOptions(&hltc6811, 0x80);
    if (status == LTC6811_OK) {
        printf("ADC options configured for high accuracy\r\n");
    } else {
        printf("ERROR: Failed to configure ADC options\r\n");
    }
}

/**
 * @brief Print cell voltage data
 */
void LTC6811_Example_PrintCellVoltages(void)
{
    printf("\r\nCell Voltages:\r\n");
    printf("==============\r\n");
    
    for (uint8_t cell = 1; cell <= 12; cell++) {
        uint16_t voltage = LTC6811_GetCellVoltage(&hltc6811, cell);
        printf("Cell %2d: %4d mV\r\n", cell, voltage);
    }
}

/**
 * @brief Print auxiliary data
 */
void LTC6811_Example_PrintAuxiliaryData(void)
{
    printf("\r\nAuxiliary Data:\r\n");
    printf("================\r\n");
    
    for (uint8_t aux = 1; aux <= 6; aux++) {
        uint16_t voltage = LTC6811_GetAuxVoltage(&hltc6811, aux);
        printf("AUX %d: %4d mV\r\n", aux, voltage);
    }
    
    /* Get temperature */
    int16_t temperature = LTC6811_GetTemperature(&hltc6811);
    printf("Temperature: %d (raw)\r\n", temperature);
}

/**
 * @brief Print status information
 */
void LTC6811_Example_PrintStatus(void)
{
    printf("\r\nStatus Information:\r\n");
    printf("===================\r\n");
    
    /* Check various status flags */
    bool is_overvoltage = LTC6811_IsOvervoltage(&hltc6811);
    bool is_undervoltage = LTC6811_IsUndervoltage(&hltc6811);
    
    printf("Overvoltage: %s\r\n", is_overvoltage ? "YES" : "NO");
    printf("Undervoltage: %s\r\n", is_undervoltage ? "YES" : "NO");
    
    /* Print raw status register values */
    printf("Status Register A:\r\n");
    for (int i = 0; i < 4; i++) {
        printf("  STATA[%d]: 0x%04X\r\n", i, hltc6811.status.STATA[i]);
    }
    
    printf("Status Register B:\r\n");
    for (int i = 0; i < 4; i++) {
        printf("  STATB[%d]: 0x%04X\r\n", i, hltc6811.status.STATB[i]);
    }
}

/**
 * @brief Example of reading a single cell voltage
 */
uint16_t LTC6811_Example_ReadSingleCell(uint8_t cell_number)
{
    if (cell_number < 1 || cell_number > 12) {
        printf("ERROR: Invalid cell number: %d\r\n", cell_number);
        return 0;
    }
    
    /* Start conversion and read all cells */
    LTC6811_StartCellVoltageConversion(&hltc6811, 0);
    LTC6811_ReadCellVoltages(&hltc6811);
    
    /* Get specific cell voltage */
    uint16_t voltage = LTC6811_GetCellVoltage(&hltc6811, cell_number);
    printf("Cell %d voltage: %d mV\r\n", cell_number, voltage);
    
    return voltage;
}

/**
 * @brief Example of continuous monitoring with alerts
 */
void LTC6811_Example_ContinuousMonitoring(void)
{
    printf("Starting continuous monitoring...\r\n");
    
    /* Configure thresholds for Li-ion cells */
    LTC6811_SetUndervoltageThreshold(&hltc6811, 3000);  // 3.0V
    LTC6811_SetOvervoltageThreshold(&hltc6811, 4200);   // 4.2V
    
    while (1) {
        /* Read all data */
        LTC6811_StartCellVoltageConversion(&hltc6811, 0);
        LTC6811_ReadCellVoltages(&hltc6811);
        LTC6811_ReadStatus(&hltc6811);
        
        /* Check for critical conditions */
        if (LTC6811_IsOvervoltage(&hltc6811)) {
            printf("CRITICAL: Overvoltage detected! Shutting down...\r\n");
            /* Add your shutdown logic here */
            break;
        }
        
        if (LTC6811_IsUndervoltage(&hltc6811)) {
            printf("WARNING: Undervoltage detected! Check battery!\r\n");
        }
        
        /* Print summary */
        printf("Monitoring - ");
        for (uint8_t cell = 1; cell <= 12; cell++) {
            uint16_t voltage = LTC6811_GetCellVoltage(&hltc6811, cell);
            printf("C%d:%d ", cell, voltage);
        }
        printf("\r\n");
        
        HAL_Delay(500); // Update every 500ms
    }
}

/**
 * @brief Example of sleep/wake functionality
 */
void LTC6811_Example_SleepWake(void)
{
    printf("Testing sleep/wake functionality...\r\n");
    
    /* Put device to sleep */
    printf("Putting LTC6811 to sleep...\r\n");
    LTC6811_Sleep(&hltc6811);
    
    /* Wait a bit */
    HAL_Delay(1000);
    
    /* Wake up device */
    printf("Waking up LTC6811...\r\n");
    LTC6811_WakeUp(&hltc6811);
    
    /* Verify device is working */
    LTC6811_Status_t status = LTC6811_ReadStatus(&hltc6811);
    if (status == LTC6811_OK) {
        printf("Device woke up successfully!\r\n");
    } else {
        printf("ERROR: Device failed to wake up\r\n");
    }
}

/**
 * @brief Example of error handling
 */
void LTC6811_Example_ErrorHandling(void)
{
    printf("Testing error handling...\r\n");
    
    /* Try to read from uninitialized device */
    LTC6811_Handle_t uninit_device;
    memset(&uninit_device, 0, sizeof(LTC6811_Handle_t));
    
    LTC6811_Status_t status = LTC6811_ReadCellVoltages(&uninit_device);
    if (status == LTC6811_ERROR_NOT_INITIALIZED) {
        printf("Correctly caught uninitialized device error\r\n");
    } else {
        printf("ERROR: Failed to catch uninitialized device error\r\n");
    }
    
    /* Try to read invalid cell number */
    uint16_t voltage = LTC6811_GetCellVoltage(&hltc6811, 13);
    if (voltage == 0) {
        printf("Correctly handled invalid cell number\r\n");
    } else {
        printf("ERROR: Failed to handle invalid cell number\r\n");
    }
}

/**
 * @brief Example of configuration validation
 */
void LTC6811_Example_ConfigValidation(void)
{
    printf("Validating configuration...\r\n");
    
    /* Read current configuration */
    LTC6811_Config_t config;
    LTC6811_Status_t status = LTC6811_ReadConfig(&hltc6811, &config);
    
    if (status == LTC6811_OK) {
        printf("Configuration validation successful\r\n");
        printf("Current settings:\r\n");
        printf("  GPIO config: 0x%02X\r\n", config.CFGR0 & 0x1F);
        printf("  ADC options: 0x%02X\r\n", (config.CFGR0 & 0x80) >> 7);
        printf("  Sleep mode: %s\r\n", (config.CFGR0 & 0x40) ? "ON" : "OFF");
        printf("  Reference: %s\r\n", (config.CFGR0 & 0x20) ? "ON" : "OFF");
    } else {
        printf("ERROR: Configuration validation failed\r\n");
    }
}