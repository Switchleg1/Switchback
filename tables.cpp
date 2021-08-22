#include "config.h"
#include "tables.h"
#include "logs.h"

//Sensors
SensorTable_t MAPSensorTable = { //ADC to PSI
  { 327,   3810},
  {1.47,   44.1},
};

SensorTable_t PUTSensorTable = { //ADC to PSI
  { 327,   3810},
  {2.94,   44.1},
};

SensorTable_t MAPRSensorTable = { //PSI to ADC
  {1.47,   44.1},
  { 327,   3810},
};

SensorTable_t PUTRSensorTable = { //PSI to ADC
  {2.94,   44.1},
  { 327,   3810},
};

//Tables
EngineTable_t BOOSTPEAKTable = { //Limit of average boost
  {1000,   1500,   2000,   2500,   3000,   3500,   4000,   4500,   5000,   5500,   6000,   6500,   7000},
  {10.0,   16.0,   17.0,   19.0,   21.0,   23.0,   24.0,   23.0,   20.0,   19.0,   18.0,   17.0,   16.0},
};

EngineTable_t BOOSTMULTTable = { //Additional boost on top of stock in percent
  {1000,   1500,   2000,   2500,   3000,   3500,   4000,   4500,   5000,   5500,   6000,   6500,   7000},
  {   0,   0.05,   0.15,   0.20,   0.25,   0.25,   0.25,   0.27,   0.35,   0.35,   0.30,   0.25,   0.20},
};

EngineTable_t BOOSTCAPTable = { //Additional boost allowed on top of stock
  {1000,   1500,   2000,   2500,   3000,   3500,   4000,   4500,   5000,   5500,   6000,   6500,   7000},
  {0.00,   1.00,   2.00,   4.00,   4.75,   5.00,   5.00,   5.00,   5.00,   5.00,   4.50,   3.00,   1.00},
};

EngineTable_t FUELTable = {  //Amount to bias fuel rail (this can be optimized due to engine flow efficiency)
  {1000,   1500,   2000,   2500,   3000,   3500,   4000,   4500,   5000,   5500,   6000,   6500,   7000},
  {0.52,   0.52,   0.52,   0.52,   0.52,   0.52,   0.52,   0.52,   0.52,   0.52,   0.52,   0.52,   0.52},
};

EngineTable_t AFRTable = { //AFR lambda adjustment
  {1000,   1500,   2000,   2500,   3000,   3500,   4000,   4500,   5000,   5500,   6000,   6500,   7000},
  {   0,   0.06,   0.10,   0.13,   0.15,   0.17,   0.19,   0.20,   0.20,   0.20,   0.20,   0.20,   0.20},
};

EngineTable_t AFRBTable = { //AFR adjustment by boost
  { 1.0,    3.0,    5.0,    7.0,    9.0,   11.0,   13.0,   15.0,   17.0,   19.0,   21.0,   23.0,   25.0},
  {0.00,   0.01,   0.05,   0.10,   0.15,   0.20,   0.30,   0.50,   0.70,   0.90,   0.95,   1.00,   1.00},
};

EngineTable_t IATTable = { //Multiply computed boost depending on air temp
  {-40,    -30,    -20,    -10,      0,     10,    20,      30,     40,     50,     60,     70,     80},
  {0.0,    0.1,    0.5,    0.8,    1.0,    1.0,   1.0,     1.0,    0.9,    0.8,    0.5,    0.1,    0.0},
};

EngineTable_t COOLANTTable = { //Multiply computed boost depending on coolant temp (safety)
  { 30,     40,     50,     60,     70,     80,    90,     100,    110,    120,    130,    140,    150},
  {0.0,    0.1,    0.3,    0.5,    0.8,    1.0,   1.0,     1.0,    1.0,    1.0,    0.8,    0.5,    0.0},
};

EngineTable_t SPEEDTable = { //Multiply computed boost depending on speed (traction)
  { 10,     20,     30,     40,     50,     60,    70,      80,     90,    100,    110,    120,    130},
  {0.0,    0.1,    0.3,    0.5,    0.8,    1.0,   1.0,     1.0,    1.0,    1.0,    1.0,    1.0,    1.0},
};

SensorTable_t* FindSensorTable(uint16_t table)
{
  switch(table)
  {
    case SEN_TABLE_MAP:
      return &MAPSensorTable;
      break;

    case SEN_TABLE_PUT:
      return &PUTSensorTable;
      break;

    case SEN_TABLE_MAPR:
      return &MAPRSensorTable;
      break;

    case SEN_TABLE_PUTR:
      return &PUTRSensorTable;
      break;
  }
}

float FindSensorTableValue(float val, uint16_t table)
{
  SensorTable_t* pTable = FindSensorTable(table);
  if(!pTable)
    return 0;

  float c_ratio = (val - (*pTable)[0][0]) / ((*pTable)[0][1] - (*pTable)[0][0]);

  return (*pTable)[1][0] + (((*pTable)[1][1] - (*pTable)[1][0]) * c_ratio);
}

float GetSensorTableValue(uint16_t pos, uint16_t table)
{
  if(pos >= 2)
    return false;

  SensorTable_t* pTable = FindSensorTable(table);
  if(!pTable)
    return false;

  return (*pTable)[1][pos];
}

uint16_t SetSensorTableValue(uint16_t pos, float val, uint16_t table, uint16_t wt)
{
  if(pos >= 2)
    return false;

  SensorTable_t* pTable = FindSensorTable(table);
  if(!pTable)
    return false;;

  (*pTable)[1][pos] = val;
  
  if(wt)
    EEPROMWriteFloat(EEPROM_SEN_TABLE(table, pos), val);

  return true;
}

uint16_t ReadSensorTable(uint16_t table)
{
  SensorTable_t* pTable = FindSensorTable(table);
  if(!pTable)
    return false;

  for(uint16_t i=0; i<2; i++)
    (*pTable)[1][i] = EEPROMReadFloat(EEPROM_SEN_TABLE(table, i));

  return true;
}

uint16_t WriteSensorTable(uint16_t table)
{
  SensorTable_t* pTable = FindSensorTable(table);
  if(!pTable)
    return false;

  for(uint16_t i=0; i<2; i++)
    EEPROMWriteFloat(EEPROM_SEN_TABLE(table, i), (*pTable)[1][i], false);

  EEPROMCommit();

  return true;
}

EngineTable_t* FindEngineTable(uint16_t table)
{
  switch(table)
  {
    case ENG_TABLE_BOOST_PEAK:
      return &BOOSTPEAKTable;
      break;
      
    case ENG_TABLE_BOOST_MULT:
      return &BOOSTMULTTable;
      break;

    case ENG_TABLE_BOOST_CAP:
      return &BOOSTCAPTable;
      break;

    case ENG_TABLE_FUEL:
      return &FUELTable;
      break;

    case ENG_TABLE_AFR:
      return &AFRTable;
      break;

    case ENG_TABLE_AFRB:
      return &AFRBTable;
      break;

    case ENG_TABLE_IAT:
      return &IATTable;
      break;

    case ENG_TABLE_COOLANT:
      return &COOLANTTable;
      break;

    case ENG_TABLE_SPEED:
      return &SPEEDTable;
      break;
  }

  return NULL;
}

float FindEngineTableValue(float val, uint16_t table)
{
  EngineTable_t* pTable = FindEngineTable(table);
  if(!pTable)
    return 0;
  
  uint16_t c1 = 1;
  while(val > (*pTable)[0][c1] && c1 < ENG_TABLE_COLUMNS-1) c1++;
  uint16_t c = c1?c1-1:0;

  float c_ratio = 0;
  if(c != c1)
    c_ratio = (val - (*pTable)[0][c]) / ((*pTable)[0][c1] - (*pTable)[0][c]);

  return (*pTable)[1][c] + (((*pTable)[1][c1] - (*pTable)[1][c]) * c_ratio);
}

float GetEngineTableValue(uint16_t pos, uint16_t table)
{
  if(pos >= ENG_TABLE_COLUMNS)
    return false;

  EngineTable_t* pTable = FindEngineTable(table);
  if(!pTable)
    return false;

  return (*pTable)[1][pos];
}

uint16_t SetEngineTableValue(uint16_t pos, float val, uint16_t table, uint16_t wt)
{
  if(pos >= ENG_TABLE_COLUMNS)
    return false;

  EngineTable_t* pTable = FindEngineTable(table);
  if(!pTable)
    return false;;

  (*pTable)[1][pos] = val;
  
  if(wt)
    EEPROMWriteFloat(EEPROM_ENG_TABLE(table, pos), val);

  return true;
}

uint16_t ReadEngineTable(uint16_t table)
{
  EngineTable_t* pTable = FindEngineTable(table);
  if(!pTable)
    return false;

  for(uint16_t i=0; i<ENG_TABLE_COLUMNS; i++)
    (*pTable)[1][i] = EEPROMReadFloat(EEPROM_ENG_TABLE(table, i));

  return true;
}

uint16_t WriteEngineTable(uint16_t table)
{
  EngineTable_t* pTable = FindEngineTable(table);
  if(!pTable)
    return false;

  for(uint16_t i=0; i<ENG_TABLE_COLUMNS; i++)
    EEPROMWriteFloat(EEPROM_ENG_TABLE(table, i), (*pTable)[1][i], false);

  EEPROMCommit();

  return true;
}
