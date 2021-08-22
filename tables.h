#ifndef TABLES_H
#define TABLES_H

//Single row tables
#define ENG_TABLE_COUNT       9
#define ENG_TABLE_COLUMNS     13
#define ENG_TABLE_BOOST_PEAK  0
#define ENG_TABLE_BOOST_MULT  1
#define ENG_TABLE_BOOST_CAP   2
#define ENG_TABLE_FUEL        3
#define ENG_TABLE_AFR         4
#define ENG_TABLE_AFRB        5
#define ENG_TABLE_IAT         6
#define ENG_TABLE_COOLANT     7
#define ENG_TABLE_SPEED       8

#define SEN_TABLE_COUNT       4
#define SEN_TABLE_MAP         0
#define SEN_TABLE_PUT         1
#define SEN_TABLE_MAPR        2
#define SEN_TABLE_PUTR        3

//Table typedefine
typedef float EngineTable_t[2][ENG_TABLE_COLUMNS];
typedef float SensorTable_t[2][2];

float     FindSensorTableValue(float val, uint16_t table);
float     GetSensorTableValue(uint16_t pos, uint16_t table);
uint16_t  SetSensorTableValue(uint16_t pos, float val, uint16_t table, uint16_t wt = false);
uint16_t  ReadSensorTable(uint16_t table);
uint16_t  WriteSensorTable(uint16_t table);

float     FindEngineTableValue(float val, uint16_t table);
float     GetEngineTableValue(uint16_t pos, uint16_t table);
uint16_t  SetEngineTableValue(uint16_t pos, float val, uint16_t table, uint16_t wt = false);
uint16_t  ReadEngineTable(uint16_t table);
uint16_t  WriteEngineTable(uint16_t table);

#endif
