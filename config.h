#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

#define FIRMWARE_NAME   "SWITCHBACK v0.1"
#define ADMIN_PASSWORD  "SWBK187"
#define SERIAL_BAUD     500000
#define MAX_STRING      256
#define RXBUFFER_COUNT  5
#define LOG_SIZE        65536
#define LOG_TRUNC_DELAY 3600000

#define DAC1_PIN        25
#define O2_ADC_PIN      27
#define ADC_CS_PIN      2         // SPI slave select
#define ADC_VREF        5000      // 5.0V Vref
#define ADC_CLK         2000000L  // SPI clock 2.0MHz
#define DAC_CLK         800000L   // I2C clock 0.8Mhz

#define SENSOR_COUNT    4
#define SENSOR_BOOST    0
#define SENSOR_MANIFOLD 1
#define SENSOR_FUEL     2
#define SENSOR_O2       3

// default timeout for a response in milliseconds
#define OBD2_DEFAULT_TIMEOUT  200
#define OBD2_RESPONSE_TIME    0

#define DEBUG
#define DEBUG_SERIAL
//#define DEBUG_BT
//#define DEBUG_SENSORS
//#define DEBUG_TASK

#define EEPROM_RESET                1       //Uncomment to reset eeprom settings
#define EEPROM_SEN_TABLE_SIZE       8
#define EEPROM_SEN_TABLE(X, Y)      EEPROM_TABLE_OFFSET+(EEPROM_SEN_TABLE_SIZE*X)+(Y*4)
#define EEPROM_ENG_TABLE_SIZE       ENG_TABLE_COLUMNS*4
#define EEPROM_ENG_TABLE(X, Y)      EEPROM_TABLE_OFFSET+(SEN_TABLE_COUNT*EEPROM_ENG_TABLE_SIZE)+(EEPROM_ENG_TABLE_SIZE*X)+(Y*4)
#define EEPROM_TABLE_OFFSET         44
#define EEPROM_SIZE                 1024
#define EEPROM_PRINT_PIDS           0
#define EEPROM_PRINT_SPEED          1
#define EEPROM_SEN_TABLE_MAP        EEPROM_SEN_TABLE(0, 0)
#define EEPROM_SEN_TABLE_PUT        EEPROM_SEN_TABLE(1, 0)
#define EEPROM_SEN_TABLE_MAPR       EEPROM_SEN_TABLE(2, 0)
#define EEPROM_SEN_TABLE_PUTR       EEPROM_SEN_TABLE(3, 0)
#define EEPROM_ENG_TABLE_BOOST_PEAK EEPROM_ENG_TABLE(0, 0)
#define EEPROM_ENG_TABLE_BOOST_MULT EEPROM_ENG_TABLE(1, 0)
#define EEPROM_ENG_TABLE_BOOST_CAP  EEPROM_ENG_TABLE(2, 0)
#define EEPROM_ENG_TABLE_FUEL       EEPROM_ENG_TABLE(3, 0)
#define EEPROM_ENG_TABLE_AFR        EEPROM_ENG_TABLE(4, 0)
#define EEPROM_ENG_TABLE_AFRB       EEPROM_ENG_TABLE(5, 0)
#define EEPROM_ENG_TABLE_IAT        EEPROM_ENG_TABLE(6, 0)
#define EEPROM_ENG_TABLE_COOLANT    EEPROM_ENG_TABLE(7, 0)
#define EEPROM_ENG_TABLE_SPEED      EEPROM_ENG_TABLE(8, 0)

void    EEPROMBegin();
int8_t  EEPROMReadByte(uint16_t address);
int16_t EEPROMReadInt(uint16_t address);
float   EEPROMReadFloat(uint16_t address);
void    EEPROMWriteByte(uint16_t address, int8_t val, uint16_t commit = true);
void    EEPROMWriteInt(uint16_t address, int16_t val, uint16_t commit = true);
void    EEPROMWriteFloat(uint16_t address, float val, uint16_t commit = true);
void    EEPROMCommit();

#endif
