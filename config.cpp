#include <EEPROM.h>
#include "config.h"
#include "logs.h"

void EEPROMBegin()
{
  LogPrint("EEPROM setup: ");
  
  EEPROM.begin(EEPROM_SIZE);
  
  LogPrintln("complete.");
}

int8_t EEPROMReadByte(uint16_t address)
{
  return EEPROM.read(address);
}

int16_t EEPROMReadInt(uint16_t address)
{
  int16_t val;
  uint8_t *bval = (uint8_t*)&val;
  bval[0] = EEPROM.read(address);
  bval[1] = EEPROM.read(address+1);

  return val;
}

float EEPROMReadFloat(uint16_t address)
{
  float val;
  uint8_t *bval = (uint8_t*)&val;
  bval[0] = EEPROM.read(address);
  bval[1] = EEPROM.read(address+1);
  bval[2] = EEPROM.read(address+2);
  bval[3] = EEPROM.read(address+3);

  return val;
}

void EEPROMWriteByte(uint16_t address, int8_t val, uint16_t commit)
{
  EEPROM.write(address, val);

  if(commit)
    EEPROM.commit();
}

void EEPROMWriteInt(uint16_t address, int16_t val, uint16_t commit)
{
  uint8_t *bval = (uint8_t*)&val;
  EEPROM.write(address, bval[0]);
  EEPROM.write(address+1, bval[1]);

  if(commit)
    EEPROM.commit();
}

void EEPROMWriteFloat(uint16_t address, float val, uint16_t commit)
{
  uint8_t *bval = (uint8_t*)&val;
  EEPROM.write(address, bval[0]);
  EEPROM.write(address+1, bval[1]);
  EEPROM.write(address+2, bval[2]);
  EEPROM.write(address+3, bval[3]);

  if(commit)
    EEPROM.commit();
}

void EEPROMCommit()
{
  EEPROM.commit();
}
