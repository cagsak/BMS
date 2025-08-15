# LTC6811 Battery Monitor Driver for STM32

A comprehensive driver for the LTC6811 multi-cell battery stack monitor IC, designed specifically for STM32 microcontrollers using the HAL library.

## Overview

The LTC6811 is a precision battery monitoring IC that can monitor up to 12 series connected battery cells with high accuracy. This driver provides a complete interface for STM32 microcontrollers to communicate with the LTC6811 via SPI.

## Features

- **Multi-cell monitoring**: Support for up to 12 battery cells
- **High accuracy**: 16-bit ADC with 1.5mV resolution
- **SPI communication**: Full SPI interface implementation
- **PEC validation**: Built-in Packet Error Code checking
- **Voltage thresholds**: Configurable overvoltage and undervoltage limits
- **GPIO support**: 5 general-purpose I/O channels
- **Temperature monitoring**: Built-in temperature sensor support
- **Sleep mode**: Power management capabilities
- **Error handling**: Comprehensive error checking and reporting

## Hardware Requirements

- STM32 microcontroller (STM32F4, STM32H7, etc.)
- LTC6811 IC
- SPI interface
- Chip select GPIO pin
- Power supply (3.3V or 5V)

## Pin Connections

| LTC6811 Pin | STM32 Connection | Description |
|-------------|------------------|-------------|
| V+          | 3.3V/5V         | Power supply |
| GND         | GND             | Ground       |
| SCLK        | SPI_SCK         | SPI clock    |
| CS          | GPIO            | Chip select  |
| SDI         | SPI_MOSI        | SPI data in  |
| SDO         | SPI_MISO        | SPI data out |
| VREF        | 3.3V            | Reference    |
| C1-C12      | Battery cells   | Cell inputs  |
| GPIO1-5     | Optional        | GPIO inputs  |

## Files

- `LTC6811.h` - Header file with all definitions and function prototypes
- `LTC6811.c` - Implementation file with all driver functions
- `LTC6811_example.c` - Comprehensive example usage
- `README.md` - This documentation file

## Quick Start

### 1. Include the driver

```c
#include "LTC6811.h"
```

### 2. Define handles

```c
SPI_HandleTypeDef hspi1;  // Your SPI handle
LTC6811_Handle_t hltc6811;
```

### 3. Initialize the device

```c
LTC6811_Status_t status = LTC6811_Init(&hltc6811, &hspi1, GPIOA, GPIO_PIN_4);
if (status != LTC6811_OK) {
    // Handle error
}
```

### 4. Read cell voltages

```c
// Start conversion
LTC6811_StartCellVoltageConversion(&hltc6811, 0);

// Read voltages
LTC6811_ReadCellVoltages(&hltc6811);

// Get specific cell voltage
uint16_t voltage = LTC6811_GetCellVoltage(&hltc6811, 1); // Cell 1
```

## API Reference

### Initialization Functions

#### `LTC6811_Init()`
Initialize the LTC6811 device.

```c
LTC6811_Status_t LTC6811_Init(LTC6811_Handle_t *hltc, 
                               SPI_HandleTypeDef *hspi,
                               GPIO_TypeDef *cs_port, 
                               uint16_t cs_pin);
```

**Parameters:**
- `hltc`: Pointer to LTC6811 handle
- `hspi`: SPI handle
- `cs_port`: Chip select port
- `cs_pin`: Chip select pin

**Returns:** `LTC6811_Status_t` - Status of initialization

#### `LTC6811_DeInit()`
Deinitialize the LTC6811 device.

```c
LTC6811_Status_t LTC6811_DeInit(LTC6811_Handle_t *hltc);
```

### Configuration Functions

#### `LTC6811_WriteConfig()`
Write configuration registers.

```c
LTC6811_Status_t LTC6811_WriteConfig(LTC6811_Handle_t *hltc, 
                                     LTC6811_Config_t *config);
```

#### `LTC6811_ReadConfig()`
Read configuration registers.

```c
LTC6811_Status_t LTC6811_ReadConfig(LTC6811_Handle_t *hltc, 
                                    LTC6811_Config_t *config);
```

#### `LTC6811_SetUndervoltageThreshold()`
Set cell undervoltage threshold.

```c
LTC6811_Status_t LTC6811_SetUndervoltageThreshold(LTC6811_Handle_t *hltc, 
                                                  uint16_t threshold);
```

**Parameters:**
- `threshold`: Threshold in millivolts (e.g., 3000 for 3.0V)

#### `LTC6811_SetOvervoltageThreshold()`
Set cell overvoltage threshold.

```c
LTC6811_Status_t LTC6811_SetOvervoltageThreshold(LTC6811_Handle_t *hltc, 
                                                 uint16_t threshold);
```

### Cell Voltage Functions

#### `LTC6811_StartCellVoltageConversion()`
Start cell voltage ADC conversion.

```c
LTC6811_Status_t LTC6811_StartCellVoltageConversion(LTC6811_Handle_t *hltc, 
                                                    uint8_t mode);
```

**Parameters:**
- `mode`: 0 = Normal, 1 = Self-test, 2 = Overlap

#### `LTC6811_ReadCellVoltages()`
Read all cell voltages.

```c
LTC6811_Status_t LTC6811_ReadCellVoltages(LTC6811_Handle_t *hltc);
```

#### `LTC6811_GetCellVoltage()`
Get specific cell voltage.

```c
uint16_t LTC6811_GetCellVoltage(LTC6811_Handle_t *hltc, uint8_t cell);
```

**Parameters:**
- `cell`: Cell number (1-12)

**Returns:** Voltage in millivolts

### Auxiliary Functions

#### `LTC6811_StartAuxConversion()`
Start auxiliary ADC conversion.

```c
LTC6811_Status_t LTC6811_StartAuxConversion(LTC6811_Handle_t *hltc, 
                                            uint8_t mode);
```

#### `LTC6811_ReadAuxiliary()`
Read auxiliary data.

```c
LTC6811_Status_t LTC6811_ReadAuxiliary(LTC6811_Handle_t *hltc);
```

#### `LTC6811_GetAuxVoltage()`
Get auxiliary voltage.

```c
uint16_t LTC6811_GetAuxVoltage(LTC6811_Handle_t *hltc, uint8_t aux);
```

### Status Functions

#### `LTC6811_ReadStatus()`
Read status registers.

```c
LTC6811_Status_t LTC6811_ReadStatus(LTC6811_Handle_t *hltc);
```

#### `LTC6811_IsOvervoltage()`
Check if any cell is overvoltage.

```c
bool LTC6811_IsOvervoltage(LTC6811_Handle_t *hltc);
```

#### `LTC6811_IsUndervoltage()`
Check if any cell is undervoltage.

```c
bool LTC6811_IsUndervoltage(LTC6811_Handle_t *hltc);
```

### Power Management

#### `LTC6811_Sleep()`
Put device to sleep mode.

```c
LTC6811_Status_t LTC6811_Sleep(LTC6811_Handle_t *hltc);
```

#### `LTC6811_WakeUp()`
Wake device from sleep mode.

```c
LTC6811_Status_t LTC6811_WakeUp(LTC6811_Handle_t *hltc);
```

## Configuration Structure

```c
typedef struct {
    uint8_t CFGR0;     // Configuration Register 0
    uint8_t CFGR1;     // Configuration Register 1
    uint8_t CFGR2;     // Configuration Register 2
    uint8_t CFGR3;     // Configuration Register 3
    uint8_t CFGR4;     // Configuration Register 4
    uint8_t CFGR5;     // Configuration Register 5
} LTC6811_Config_t;
```

### Configuration Register Bits

#### CFGR0 (Configuration Register 0)
- `GPIO1-GPIO5`: GPIO enable bits
- `REFON`: Reference enable
- `SWTRD`: Software timer reset
- `ADCOPT`: ADC options

#### CFGR1-CFGR2 (Undervoltage Threshold)
- 16-bit undervoltage threshold value

#### CFGR3-CFGR4 (Overvoltage Threshold)
- 16-bit overvoltage threshold value

## Error Codes

```c
typedef enum {
    LTC6811_OK = 0,                    // Operation successful
    LTC6811_ERROR_TIMEOUT,             // Operation timeout
    LTC6811_ERROR_PEC,                 // PEC validation failed
    LTC6811_ERROR_SPI,                 // SPI communication error
    LTC6811_ERROR_INVALID_PARAM,       // Invalid parameter
    LTC6811_ERROR_NOT_INITIALIZED      // Device not initialized
} LTC6811_Status_t;
```

## Example Usage

### Basic Cell Voltage Reading

```c
#include "LTC6811.h"

LTC6811_Handle_t hltc6811;

void setup_battery_monitor(void) {
    // Initialize LTC6811
    LTC6811_Init(&hltc6811, &hspi1, GPIOA, GPIO_PIN_4);
    
    // Set voltage thresholds
    LTC6811_SetUndervoltageThreshold(&hltc6811, 3000);  // 3.0V
    LTC6811_SetOvervoltageThreshold(&hltc6811, 4200);   // 4.2V
}

void read_battery_voltages(void) {
    // Start conversion
    LTC6811_StartCellVoltageConversion(&hltc6811, 0);
    
    // Read all voltages
    LTC6811_ReadCellVoltages(&hltc6811);
    
    // Print voltages
    for (int i = 1; i <= 12; i++) {
        uint16_t voltage = LTC6811_GetCellVoltage(&hltc6811, i);
        printf("Cell %d: %d mV\n", i, voltage);
    }
}
```

### Continuous Monitoring

```c
void continuous_monitoring(void) {
    while (1) {
        // Read all data
        LTC6811_StartCellVoltageConversion(&hltc6811, 0);
        LTC6811_ReadCellVoltages(&hltc6811);
        LTC6811_ReadStatus(&hltc6811);
        
        // Check for violations
        if (LTC6811_IsOvervoltage(&hltc6811)) {
            printf("OVERVOLTAGE DETECTED!\n");
            // Handle overvoltage
        }
        
        if (LTC6811_IsUndervoltage(&hltc6811)) {
            printf("UNDERVOLTAGE DETECTED!\n");
            // Handle undervoltage
        }
        
        HAL_Delay(1000);  // Wait 1 second
    }
}
```

## Integration with STM32CubeMX

### 1. Configure SPI
- Enable SPI peripheral
- Set mode to Master
- Configure clock polarity and phase
- Set baud rate (typically 1MHz or less for LTC6811)

### 2. Configure GPIO
- Set chip select pin as GPIO Output
- Configure as Push Pull, No Pull-up/Pull-down

### 3. Generate Code
- Generate initialization code
- Copy the generated handles to your main file

### 4. Include Driver
- Add LTC6811 files to your project
- Include the header in your main file

## Timing Requirements

The LTC6811 has specific timing requirements:

- **Wake-up time**: 300μs minimum
- **Command transmission**: 100μs minimum
- **Data transmission**: 100μs minimum
- **Conversion time**: 1-10ms depending on settings

The driver includes built-in delays to meet these requirements.

## PEC (Packet Error Code)

The driver automatically calculates and verifies PEC for all communications. PEC is a 15-bit CRC that ensures data integrity.

## Troubleshooting

### Common Issues

1. **SPI Communication Errors**
   - Check SPI configuration (mode, baud rate)
   - Verify chip select timing
   - Check power supply voltage

2. **PEC Errors**
   - Verify SPI timing
   - Check for noise on SPI lines
   - Ensure proper power supply

3. **Initialization Failures**
   - Check SPI handle initialization
   - Verify GPIO configuration
   - Check power supply

### Debug Tips

1. Use the example code to verify basic functionality
2. Check SPI signals with oscilloscope
3. Verify power supply voltages
4. Use status functions to check device state

## Performance Considerations

- **Update Rate**: Typical update rate is 1-10Hz
- **Power Consumption**: ~1mA active, ~1μA sleep
- **Accuracy**: ±1.5mV typical
- **Temperature Range**: -40°C to +125°C

## License

This driver is provided as-is for educational and development purposes. Please refer to the LTC6811 datasheet for detailed specifications and requirements.

## Support

For issues or questions:
1. Check the example code
2. Verify hardware connections
3. Review STM32 SPI configuration
4. Consult LTC6811 datasheet

## Version History

- **v1.0**: Initial release with full functionality
  - Basic initialization and configuration
  - Cell voltage reading
  - Auxiliary data reading
  - Status monitoring
  - Error handling
  - Example code