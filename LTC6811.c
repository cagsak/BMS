/**
 * @file LTC6811.c
 * @brief LTC6811 Multi-Cell Battery Stack Monitor Driver Implementation
 * @version 1.0
 * @date 2024
 */

#include "LTC6811.h"
#include <string.h>

/* Private function prototypes */
static LTC6811_Status_t LTC6811_SPI_Transmit(LTC6811_Handle_t *hltc, uint8_t *data, uint16_t size);
static LTC6811_Status_t LTC6811_SPI_Receive(LTC6811_Handle_t *hltc, uint8_t *data, uint16_t size);
static LTC6811_Status_t LTC6811_SPI_TransmitReceive(LTC6811_Handle_t *hltc, uint8_t *tx_data, uint8_t *rx_data, uint16_t size);
static void LTC6811_SendCommand(LTC6811_Handle_t *hltc, uint16_t command);

/* PEC15 table for CRC calculation */
static const uint16_t PEC15_TABLE[256] = {
    0x0000, 0xC599, 0xCEF3, 0x0B6A, 0xD8E7, 0x1D7E, 0x1614, 0xD38D,
    0xF1CF, 0x3456, 0x3F3C, 0xFAA5, 0x2928, 0xECB1, 0xE7DB, 0x2242,
    0xE39F, 0x2606, 0x2D6C, 0xE8F5, 0x3B78, 0xFEE1, 0xF58B, 0x3012,
    0x1250, 0xD7C9, 0xDCA3, 0x193A, 0xCAB7, 0x0F2E, 0x0444, 0xC1DD,
    0xC73F, 0x02A6, 0x09CC, 0xCC55, 0x1FD8, 0xDA41, 0xD12B, 0x14B2,
    0x36F0, 0xF369, 0xF803, 0x3D9A, 0xEE17, 0x2B8E, 0x20E4, 0xE57D,
    0x24A0, 0xE139, 0xEA53, 0x2FCA, 0xFC47, 0x39DE, 0x32B4, 0xF72D,
    0xD56F, 0x10F6, 0x1B9C, 0xDE05, 0x0D88, 0xC811, 0xC37B, 0x06E2,
    0x8E7F, 0x4BE6, 0x408C, 0x8515, 0x5698, 0x9301, 0x986B, 0x5DF2,
    0x7FB0, 0xBA29, 0xB143, 0x74DA, 0xA757, 0x62CE, 0x69A4, 0xAC3D,
    0x6DE0, 0xA879, 0xA313, 0x668A, 0xB507, 0x709E, 0x7BF4, 0xBE6D,
    0x9C2F, 0x59B6, 0x52DC, 0x9745, 0x44C8, 0x8151, 0x8A3B, 0x4FA2,
    0x4940, 0x8CD9, 0x87B3, 0x422A, 0x91A7, 0x543E, 0x5F54, 0x9ACD,
    0xB88F, 0x7D16, 0x767C, 0xB3E5, 0x6068, 0xA5F1, 0xAE9B, 0x6B02,
    0xAADF, 0x6F46, 0x642C, 0xA1B5, 0x7238, 0xB7A1, 0xBCCB, 0x7952,
    0x5B10, 0x9E89, 0x95E3, 0x507A, 0x83F7, 0x466E, 0x4D04, 0x889D,
    0x1CFF, 0xD966, 0xD20C, 0x1795, 0xC418, 0x0181, 0x0AEB, 0xCF72,
    0xED30, 0x28A9, 0x23C3, 0xE65A, 0x35D7, 0xF04E, 0xFB24, 0x3EBD,
    0xFF60, 0x3AF9, 0x3193, 0xF40A, 0x2787, 0xE21E, 0xE974, 0x2CED,
    0x0EAF, 0xCB36, 0xC05C, 0x05C5, 0xD648, 0x13D1, 0x18BB, 0xDD22,
    0xDBD0, 0x1E49, 0x1523, 0xD0BA, 0x0337, 0xC6AE, 0xCDC4, 0x085D,
    0x2A1F, 0xEF86, 0xE4EC, 0x2175, 0xF2F8, 0x3761, 0x3C0B, 0xF992,
    0x384F, 0xFDD6, 0xF6BC, 0x3325, 0xE0A8, 0x2531, 0x2E5B, 0xEBC2,
    0xC980, 0x0C19, 0x0773, 0xC2EA, 0x1167, 0xD4FE, 0xDF94, 0x1A0D,
    0x9280, 0x5719, 0x5C73, 0x99EA, 0x4A67, 0x8FFE, 0x8494, 0x410D,
    0x634F, 0xA6D6, 0xADBC, 0x6825, 0xBBA8, 0x7E31, 0x755B, 0xB0C2,
    0x711F, 0xB486, 0xBFEC, 0x7A75, 0xA9F8, 0x6C61, 0x670B, 0xA292,
    0x80D0, 0x4549, 0x4E23, 0x8BBA, 0x5837, 0x9DAE, 0x96C4, 0x535D,
    0x55BF, 0x9026, 0x9B4C, 0x5ED5, 0x8D58, 0x48C1, 0x43AB, 0x8632,
    0xA470, 0x61E9, 0x6A83, 0xAF1A, 0x7C97, 0xB90E, 0xB264, 0x77FD,
    0xB620, 0x73B9, 0x78D3, 0xBD4A, 0x6EC7, 0xAB5E, 0xA034, 0x65AD,
    0x47EF, 0x8276, 0x891C, 0x4C85, 0x9F08, 0x5A91, 0x51FB, 0x9462
};

/**
 * @brief Initialize LTC6811 device
 */
LTC6811_Status_t LTC6811_Init(LTC6811_Handle_t *hltc, 
                               SPI_HandleTypeDef *hspi,
                               GPIO_TypeDef *cs_port, 
                               uint16_t cs_pin)
{
    if (hltc == NULL || hspi == NULL) {
        return LTC6811_ERROR_INVALID_PARAM;
    }
    
    /* Initialize handle */
    hltc->hspi = hspi;
    hltc->CS_Port = cs_port;
    hltc->CS_Pin = cs_pin;
    hltc->isInitialized = false;
    
    /* Set default configuration */
    hltc->config.CFGR0 = 0x00;  // Default GPIO and reference settings
    hltc->config.CFGR1 = 0x00;  // Default undervoltage threshold
    hltc->config.CFGR2 = 0x00;  // Default overvoltage threshold
    hltc->config.CFGR3 = 0x00;  // Default configuration
    hltc->config.CFGR4 = 0x00;  // Default configuration
    hltc->config.CFGR5 = 0x00;  // Default configuration
    
    /* Initialize chip select */
    LTC6811_CS_High(hltc);
    
    /* Wake up device */
    LTC6811_Status_t status = LTC6811_WakeUp(hltc);
    if (status != LTC6811_OK) {
        return status;
    }
    
    /* Write initial configuration */
    status = LTC6811_WriteConfig(hltc, &hltc->config);
    if (status != LTC6811_OK) {
        return status;
    }
    
    hltc->isInitialized = true;
    return LTC6811_OK;
}

/**
 * @brief Deinitialize LTC6811 device
 */
LTC6811_Status_t LTC6811_DeInit(LTC6811_Handle_t *hltc)
{
    if (hltc == NULL) {
        return LTC6811_ERROR_INVALID_PARAM;
    }
    
    if (!hltc->isInitialized) {
        return LTC6811_ERROR_NOT_INITIALIZED;
    }
    
    /* Put device to sleep */
    LTC6811_Sleep(hltc);
    
    /* Clear handle */
    hltc->isInitialized = false;
    hltc->hspi = NULL;
    
    return LTC6811_OK;
}

/**
 * @brief Write configuration registers
 */
LTC6811_Status_t LTC6811_WriteConfig(LTC6811_Handle_t *hltc, 
                                     LTC6811_Config_t *config)
{
    if (hltc == NULL || config == NULL) {
        return LTC6811_ERROR_INVALID_PARAM;
    }
    
    if (!hltc->isInitialized) {
        return LTC6811_ERROR_NOT_INITIALIZED;
    }
    
    uint8_t tx_data[LTC6811_CMD_LENGTH + 6 + LTC6811_PEC_LENGTH];
    uint8_t *data_ptr = tx_data;
    
    /* Command */
    *data_ptr++ = (LTC6811_CMD_WRCFGA >> 8) & 0xFF;
    *data_ptr++ = LTC6811_CMD_WRCFGA & 0xFF;
    *data_ptr++ = 0x00;
    *data_ptr++ = 0x00;
    
    /* Configuration data */
    *data_ptr++ = config->CFGR0;
    *data_ptr++ = config->CFGR1;
    *data_ptr++ = config->CFGR2;
    *data_ptr++ = config->CFGR3;
    *data_ptr++ = config->CFGR4;
    *data_ptr++ = config->CFGR5;
    
    /* Calculate and add PEC */
    uint16_t pec = LTC6811_CalculatePEC(tx_data, 10);
    *data_ptr++ = (pec >> 8) & 0xFF;
    *data_ptr++ = pec & 0xFF;
    
    /* Send command */
    LTC6811_SendCommand(hltc, LTC6811_CMD_WRCFGA);
    
    /* Transmit data */
    LTC6811_Status_t status = LTC6811_SPI_Transmit(hltc, tx_data, 12);
    if (status == LTC6811_OK) {
        /* Update local configuration */
        memcpy(&hltc->config, config, sizeof(LTC6811_Config_t));
    }
    
    return status;
}

/**
 * @brief Read configuration registers
 */
LTC6811_Status_t LTC6811_ReadConfig(LTC6811_Handle_t *hltc, 
                                    LTC6811_Config_t *config)
{
    if (hltc == NULL || config == NULL) {
        return LTC6811_ERROR_INVALID_PARAM;
    }
    
    if (!hltc->isInitialized) {
        return LTC6811_ERROR_NOT_INITIALIZED;
    }
    
    uint8_t tx_data[LTC6811_CMD_LENGTH];
    uint8_t rx_data[6 + LTC6811_PEC_LENGTH];
    
    /* Prepare command */
    tx_data[0] = (LTC6811_CMD_RDCFGA >> 8) & 0xFF;
    tx_data[1] = LTC6811_CMD_RDCFGA & 0xFF;
    tx_data[2] = 0x00;
    tx_data[3] = 0x00;
    
    /* Send command */
    LTC6811_SendCommand(hltc, LTC6811_CMD_RDCFGA);
    
    /* Transmit command and receive data */
    LTC6811_Status_t status = LTC6811_SPI_TransmitReceive(hltc, tx_data, rx_data, 8);
    if (status != LTC6811_OK) {
        return status;
    }
    
    /* Verify PEC */
    uint16_t received_pec = (rx_data[6] << 8) | rx_data[7];
    if (!LTC6811_VerifyPEC(rx_data, 6, received_pec)) {
        return LTC6811_ERROR_PEC;
    }
    
    /* Copy configuration data */
    config->CFGR0 = rx_data[0];
    config->CFGR1 = rx_data[1];
    config->CFGR2 = rx_data[2];
    config->CFGR3 = rx_data[3];
    config->CFGR4 = rx_data[4];
    config->CFGR5 = rx_data[5];
    
    return LTC6811_OK;
}

/**
 * @brief Start cell voltage conversion
 */
LTC6811_Status_t LTC6811_StartCellVoltageConversion(LTC6811_Handle_t *hltc, 
                                                    uint8_t mode)
{
    if (hltc == NULL) {
        return LTC6811_ERROR_INVALID_PARAM;
    }
    
    if (!hltc->isInitialized) {
        return LTC6811_ERROR_NOT_INITIALIZED;
    }
    
    uint16_t command;
    switch (mode) {
        case 0: // Normal
            command = LTC6811_CMD_ADCV;
            break;
        case 1: // Self-test
            command = LTC6811_CMD_CVST;
            break;
        case 2: // Overlap
            command = LTC6811_CMD_ADOL;
            break;
        default:
            return LTC6811_ERROR_INVALID_PARAM;
    }
    
    /* Send command */
    LTC6811_SendCommand(hltc, command);
    
    /* Wait for conversion to complete */
    LTC6811_DelayUs(1000); // Adjust based on your ADC settings
    
    return LTC6811_OK;
}

/**
 * @brief Read cell voltages
 */
LTC6811_Status_t LTC6811_ReadCellVoltages(LTC6811_Handle_t *hltc)
{
    if (hltc == NULL) {
        return LTC6811_ERROR_INVALID_PARAM;
    }
    
    if (!hltc->isInitialized) {
        return LTC6811_ERROR_NOT_INITIALIZED;
    }
    
    uint8_t tx_data[LTC6811_CMD_LENGTH];
    uint8_t rx_data[12 + LTC6811_PEC_LENGTH];
    
    /* Read Cell Voltage Register A (Cells 1-6) */
    tx_data[0] = (LTC6811_CMD_RDCVA >> 8) & 0xFF;
    tx_data[1] = LTC6811_CMD_RDCVA & 0xFF;
    tx_data[2] = 0x00;
    tx_data[3] = 0x00;
    
    LTC6811_SendCommand(hltc, LTC6811_CMD_RDCVA);
    
    LTC6811_Status_t status = LTC6811_SPI_TransmitReceive(hltc, tx_data, rx_data, 14);
    if (status != LTC6811_OK) {
        return status;
    }
    
    /* Verify PEC */
    uint16_t received_pec = (rx_data[12] << 8) | rx_data[13];
    if (!LTC6811_VerifyPEC(rx_data, 12, received_pec)) {
        return LTC6811_ERROR_PEC;
    }
    
    /* Parse cell voltages A (Cells 1-6) */
    for (int i = 0; i < 6; i++) {
        hltc->cellVoltage.CVA[i] = (rx_data[i*2] << 8) | rx_data[i*2 + 1];
    }
    
    /* Read Cell Voltage Register B (Cells 7-12) */
    tx_data[0] = (LTC6811_CMD_RDCVB >> 8) & 0xFF;
    tx_data[1] = LTC6811_CMD_RDCVB & 0xFF;
    
    LTC6811_SendCommand(hltc, LTC6811_CMD_RDCVB);
    
    status = LTC6811_SPI_TransmitReceive(hltc, tx_data, rx_data, 14);
    if (status != LTC6811_OK) {
        return status;
    }
    
    /* Verify PEC */
    received_pec = (rx_data[12] << 8) | rx_data[13];
    if (!LTC6811_VerifyPEC(rx_data, 12, received_pec)) {
        return LTC6811_ERROR_PEC;
    }
    
    /* Parse cell voltages B (Cells 7-12) */
    for (int i = 0; i < 6; i++) {
        hltc->cellVoltage.CVB[i] = (rx_data[i*2] << 8) | rx_data[i*2 + 1];
    }
    
    return LTC6811_OK;
}

/**
 * @brief Start auxiliary conversion
 */
LTC6811_Status_t LTC6811_StartAuxConversion(LTC6811_Handle_t *hltc, 
                                            uint8_t mode)
{
    if (hltc == NULL) {
        return LTC6811_ERROR_INVALID_PARAM;
    }
    
    if (!hltc->isInitialized) {
        return LTC6811_ERROR_NOT_INITIALIZED;
    }
    
    uint16_t command = (mode == 1) ? LTC6811_CMD_ADAXD : LTC6811_CMD_ADAX;
    
    /* Send command */
    LTC6811_SendCommand(hltc, command);
    
    /* Wait for conversion to complete */
    LTC6811_DelayUs(1000);
    
    return LTC6811_OK;
}

/**
 * @brief Read auxiliary data
 */
LTC6811_Status_t LTC6811_ReadAuxiliary(LTC6811_Handle_t *hltc)
{
    if (hltc == NULL) {
        return LTC6811_ERROR_INVALID_PARAM;
    }
    
    if (!hltc->isInitialized) {
        return LTC6811_ERROR_NOT_INITIALIZED;
    }
    
    uint8_t tx_data[LTC6811_CMD_LENGTH];
    uint8_t rx_data[12 + LTC6811_PEC_LENGTH];
    
    /* Read Auxiliary Register A */
    tx_data[0] = (LTC6811_CMD_RDAUXA >> 8) & 0xFF;
    tx_data[1] = LTC6811_CMD_RDAUXA & 0xFF;
    tx_data[2] = 0x00;
    tx_data[3] = 0x00;
    
    LTC6811_SendCommand(hltc, LTC6811_CMD_RDAUXA);
    
    LTC6811_Status_t status = LTC6811_SPI_TransmitReceive(hltc, tx_data, rx_data, 14);
    if (status != LTC6811_OK) {
        return status;
    }
    
    /* Verify PEC */
    uint16_t received_pec = (rx_data[12] << 8) | rx_data[13];
    if (!LTC6811_VerifyPEC(rx_data, 12, received_pec)) {
        return LTC6811_ERROR_PEC;
    }
    
    /* Parse auxiliary data A */
    for (int i = 0; i < 6; i++) {
        hltc->auxiliary.AUXA[i] = (rx_data[i*2] << 8) | rx_data[i*2 + 1];
    }
    
    /* Read Auxiliary Register B */
    tx_data[0] = (LTC6811_CMD_RDAUXB >> 8) & 0xFF;
    tx_data[1] = LTC6811_CMD_RDAUXB & 0xFF;
    
    LTC6811_SendCommand(hltc, LTC6811_CMD_RDAUXB);
    
    status = LTC6811_SPI_TransmitReceive(hltc, tx_data, rx_data, 14);
    if (status != LTC6811_OK) {
        return status;
    }
    
    /* Verify PEC */
    received_pec = (rx_data[12] << 8) | rx_data[13];
    if (!LTC6811_VerifyPEC(rx_data, 12, received_pec)) {
        return LTC6811_ERROR_PEC;
    }
    
    /* Parse auxiliary data B */
    for (int i = 0; i < 6; i++) {
        hltc->auxiliary.AUXB[i] = (rx_data[i*2] << 8) | rx_data[i*2 + 1];
    }
    
    return LTC6811_OK;
}

/**
 * @brief Read status registers
 */
LTC6811_Status_t LTC6811_ReadStatus(LTC6811_Handle_t *hltc)
{
    if (hltc == NULL) {
        return LTC6811_ERROR_INVALID_PARAM;
    }
    
    if (!hltc->isInitialized) {
        return LTC6811_ERROR_NOT_INITIALIZED;
    }
    
    uint8_t tx_data[LTC6811_CMD_LENGTH];
    uint8_t rx_data[8 + LTC6811_PEC_LENGTH];
    
    /* Read Status Register A */
    tx_data[0] = (LTC6811_CMD_RDSTATA >> 8) & 0xFF;
    tx_data[1] = LTC6811_CMD_RDSTATA & 0xFF;
    tx_data[2] = 0x00;
    tx_data[3] = 0x00;
    
    LTC6811_SendCommand(hltc, LTC6811_CMD_RDSTATA);
    
    LTC6811_Status_t status = LTC6811_SPI_TransmitReceive(hltc, tx_data, rx_data, 10);
    if (status != LTC6811_OK) {
        return status;
    }
    
    /* Verify PEC */
    uint16_t received_pec = (rx_data[8] << 8) | rx_data[9];
    if (!LTC6811_VerifyPEC(rx_data, 8, received_pec)) {
        return LTC6811_ERROR_PEC;
    }
    
    /* Parse status data A */
    for (int i = 0; i < 4; i++) {
        hltc->status.STATA[i] = (rx_data[i*2] << 8) | rx_data[i*2 + 1];
    }
    
    /* Read Status Register B */
    tx_data[0] = (LTC6811_CMD_RDSTATB >> 8) & 0xFF;
    tx_data[1] = LTC6811_CMD_RDSTATB & 0xFF;
    
    LTC6811_SendCommand(hltc, LTC6811_CMD_RDSTATB);
    
    status = LTC6811_SPI_TransmitReceive(hltc, tx_data, rx_data, 10);
    if (status != LTC6811_OK) {
        return status;
    }
    
    /* Verify PEC */
    received_pec = (rx_data[8] << 8) | rx_data[9];
    if (!LTC6811_VerifyPEC(rx_data, 8, received_pec)) {
        return LTC6811_ERROR_PEC;
    }
    
    /* Parse status data B */
    for (int i = 0; i < 4; i++) {
        hltc->status.STATB[i] = (rx_data[i*2] << 8) | rx_data[i*2 + 1];
    }
    
    return LTC6811_OK;
}

/**
 * @brief Get cell voltage in millivolts
 */
uint16_t LTC6811_GetCellVoltage(LTC6811_Handle_t *hltc, uint8_t cell)
{
    if (hltc == NULL || cell < 1 || cell > 12) {
        return 0;
    }
    
    if (!hltc->isInitialized) {
        return 0;
    }
    
    uint16_t voltage;
    if (cell <= 6) {
        voltage = hltc->cellVoltage.CVA[cell - 1];
    } else {
        voltage = hltc->cellVoltage.CVB[cell - 7];
    }
    
    /* Convert to millivolts (LTC6811 resolution is typically 1.5mV) */
    return voltage * 15 / 10;
}

/**
 * @brief Get auxiliary voltage in millivolts
 */
uint16_t LTC6811_GetAuxVoltage(LTC6811_Handle_t *hltc, uint8_t aux)
{
    if (hltc == NULL || aux < 1 || aux > 6) {
        return 0;
    }
    
    if (!hltc->isInitialized) {
        return 0;
    }
    
    uint16_t voltage;
    if (aux <= 3) {
        voltage = hltc->auxiliary.AUXA[aux - 1];
    } else {
        voltage = hltc->auxiliary.AUXB[aux - 4];
    }
    
    /* Convert to millivolts */
    return voltage * 15 / 10;
}

/**
 * @brief Calculate PEC (Packet Error Code)
 */
uint16_t LTC6811_CalculatePEC(uint8_t *data, uint8_t len)
{
    uint16_t pec = 0x0010; // Initial value
    
    for (uint8_t i = 0; i < len; i++) {
        pec = ((pec << 8) & 0xFFFF) ^ PEC15_TABLE[((pec >> 7) ^ data[i]) & 0xFF];
    }
    
    return pec;
}

/**
 * @brief Verify PEC
 */
bool LTC6811_VerifyPEC(uint8_t *data, uint8_t len, uint16_t pec)
{
    uint16_t calculated_pec = LTC6811_CalculatePEC(data, len);
    return (calculated_pec == pec);
}

/**
 * @brief Set cell undervoltage threshold
 */
LTC6811_Status_t LTC6811_SetUndervoltageThreshold(LTC6811_Handle_t *hltc, 
                                                  uint16_t threshold)
{
    if (hltc == NULL) {
        return LTC6811_ERROR_INVALID_PARAM;
    }
    
    if (!hltc->isInitialized) {
        return LTC6811_ERROR_NOT_INITIALIZED;
    }
    
    /* Convert threshold to register value */
    uint16_t reg_value = threshold * 10 / 15;
    
    /* Update configuration */
    hltc->config.CFGR1 = reg_value & 0xFF;
    hltc->config.CFGR2 = (reg_value >> 8) & 0xFF;
    
    /* Write configuration */
    return LTC6811_WriteConfig(hltc, &hltc->config);
}

/**
 * @brief Set cell overvoltage threshold
 */
LTC6811_Status_t LTC6811_SetOvervoltageThreshold(LTC6811_Handle_t *hltc, 
                                                 uint16_t threshold)
{
    if (hltc == NULL) {
        return LTC6811_ERROR_INVALID_PARAM;
    }
    
    if (!hltc->isInitialized) {
        return LTC6811_ERROR_NOT_INITIALIZED;
    }
    
    /* Convert threshold to register value */
    uint16_t reg_value = threshold * 10 / 15;
    
    /* Update configuration */
    hltc->config.CFGR3 = reg_value & 0xFF;
    hltc->config.CFGR4 = (reg_value >> 8) & 0xFF;
    
    /* Write configuration */
    return LTC6811_WriteConfig(hltc, &hltc->config);
}

/**
 * @brief Enable/disable GPIO channels
 */
LTC6811_Status_t LTC6811_SetGPIOConfig(LTC6811_Handle_t *hltc, 
                                       uint8_t gpio_mask)
{
    if (hltc == NULL) {
        return LTC6811_ERROR_INVALID_PARAM;
    }
    
    if (!hltc->isInitialized) {
        return LTC6811_ERROR_NOT_INITIALIZED;
    }
    
    /* Update GPIO configuration */
    hltc->config.CFGR0 = (hltc->config.CFGR0 & 0xE0) | (gpio_mask & 0x1F);
    
    /* Write configuration */
    return LTC6811_WriteConfig(hltc, &hltc->config);
}

/**
 * @brief Set ADC conversion options
 */
LTC6811_Status_t LTC6811_SetADCOptions(LTC6811_Handle_t *hltc, 
                                       uint8_t adc_opt)
{
    if (hltc == NULL) {
        return LTC6811_ERROR_INVALID_PARAM;
    }
    
    if (!hltc->isInitialized) {
        return LTC6811_ERROR_NOT_INITIALIZED;
    }
    
    /* Update ADC options */
    hltc->config.CFGR0 = (hltc->config.CFGR0 & 0x7F) | (adc_opt & 0x80);
    
    /* Write configuration */
    return LTC6811_WriteConfig(hltc, &hltc->config);
}

/**
 * @brief Wake up LTC6811 from sleep mode
 */
LTC6811_Status_t LTC6811_WakeUp(LTC6811_Handle_t *hltc)
{
    if (hltc == NULL) {
        return LTC6811_ERROR_INVALID_PARAM;
    }
    
    /* Send wake-up command by toggling CS */
    LTC6811_CS_Low(hltc);
    LTC6811_DelayUs(LTC6811_TWAKE);
    LTC6811_CS_High(hltc);
    
    return LTC6811_OK;
}

/**
 * @brief Put LTC6811 to sleep mode
 */
LTC6811_Status_t LTC6811_Sleep(LTC6811_Handle_t *hltc)
{
    if (hltc == NULL) {
        return LTC6811_ERROR_INVALID_PARAM;
    }
    
    /* Set sleep bit in configuration */
    hltc->config.CFGR0 |= 0x40; // Set sleep bit
    
    /* Write configuration */
    return LTC6811_WriteConfig(hltc, &hltc->config);
}

/**
 * @brief Get device temperature
 */
int16_t LTC6811_GetTemperature(LTC6811_Handle_t *hltc)
{
    if (hltc == NULL || !hltc->isInitialized) {
        return 0;
    }
    
    /* Temperature is stored in status register */
    uint16_t temp_raw = hltc->status.STATA[1];
    
    /* Convert to temperature (this conversion depends on your specific setup) */
    /* For now, return raw value - you may need to implement proper conversion */
    return (int16_t)temp_raw;
}

/**
 * @brief Check if any cell is overvoltage
 */
bool LTC6811_IsOvervoltage(LTC6811_Handle_t *hltc)
{
    if (hltc == NULL || !hltc->isInitialized) {
        return false;
    }
    
    /* Check status register for overvoltage flag */
    return (hltc->status.STATA[0] & 0x01) != 0;
}

/**
 * @brief Check if any cell is undervoltage
 */
bool LTC6811_IsUndervoltage(LTC6811_Handle_t *hltc)
{
    if (hltc == NULL || !hltc->isInitialized) {
        return false;
    }
    
    /* Check status register for undervoltage flag */
    return (hltc->status.STATA[0] & 0x02) != 0;
}

/* Private Functions */

/**
 * @brief Send command to LTC6811
 */
static void LTC6811_SendCommand(LTC6811_Handle_t *hltc, uint16_t command)
{
    uint8_t cmd_data[LTC6811_CMD_LENGTH];
    
    cmd_data[0] = (command >> 8) & 0xFF;
    cmd_data[1] = command & 0xFF;
    cmd_data[2] = 0x00;
    cmd_data[3] = 0x00;
    
    LTC6811_CS_Low(hltc);
    LTC6811_DelayUs(LTC6811_TWRITE);
    
    LTC6811_SPI_Transmit(hltc, cmd_data, LTC6811_CMD_LENGTH);
    
    LTC6811_DelayUs(LTC6811_TIDLE);
    LTC6811_CS_High(hltc);
}

/**
 * @brief SPI transmit function
 */
static LTC6811_Status_t LTC6811_SPI_Transmit(LTC6811_Handle_t *hltc, 
                                             uint8_t *data, 
                                             uint16_t size)
{
    if (HAL_SPI_Transmit(hltc->hspi, data, size, 1000) != HAL_OK) {
        return LTC6811_ERROR_SPI;
    }
    return LTC6811_OK;
}

/**
 * @brief SPI receive function
 */
static LTC6811_Status_t LTC6811_SPI_Receive(LTC6811_Handle_t *hltc, 
                                            uint8_t *data, 
                                            uint16_t size)
{
    if (HAL_SPI_Receive(hltc->hspi, data, size, 1000) != HAL_OK) {
        return LTC6811_ERROR_SPI;
    }
    return LTC6811_OK;
}

/**
 * @brief SPI transmit and receive function
 */
static LTC6811_Status_t LTC6811_SPI_TransmitReceive(LTC6811_Handle_t *hltc, 
                                                    uint8_t *tx_data, 
                                                    uint8_t *rx_data, 
                                                    uint16_t size)
{
    if (HAL_SPI_TransmitReceive(hltc->hspi, tx_data, rx_data, size, 1000) != HAL_OK) {
        return LTC6811_ERROR_SPI;
    }
    return LTC6811_OK;
}

/* Utility Functions */

/**
 * @brief Microsecond delay
 */
void LTC6811_DelayUs(uint32_t microseconds)
{
    /* This should be implemented based on your STM32 setup */
    /* You can use HAL_Delay for millisecond delays or implement a microsecond delay */
    /* For now, using a simple loop - adjust based on your clock frequency */
    volatile uint32_t i;
    for (i = 0; i < microseconds * 100; i++) {
        __NOP();
    }
}

/**
 * @brief Set chip select low
 */
void LTC6811_CS_Low(LTC6811_Handle_t *hltc)
{
    if (hltc != NULL) {
        HAL_GPIO_WritePin(hltc->CS_Port, hltc->CS_Pin, GPIO_PIN_RESET);
    }
}

/**
 * @brief Set chip select high
 */
void LTC6811_CS_High(LTC6811_Handle_t *hltc)
{
    if (hltc != NULL) {
        HAL_GPIO_WritePin(hltc->CS_Port, hltc->CS_Pin, GPIO_PIN_SET);
    }
}
