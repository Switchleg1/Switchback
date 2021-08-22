#ifndef SENSORS_H
#define SENSORS_H

#include "config.h"

typedef struct
{
    int16_t   value;
    int16_t   offset;
    uint16_t  raw;
} SENSOR_T;

void      SensorsBegin();
void      SensorsReadLoop(void* parameter);
void      SensorsWriteLoop(void* parameter);
void      SensorsWriteSpeed(uint16_t active);
uint16_t  SensorsWriteSpeed();

extern volatile SENSOR_T Sensors[SENSOR_COUNT];

#endif
