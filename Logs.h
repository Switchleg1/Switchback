#ifndef BTSERIAL_H
#define BTSERIAL_H

#include "config.h"

#ifdef DEBUG_BT
#include "BluetoothSerial.h"
extern BluetoothSerial SerialBT;
#endif


void LogBegin();
void LogLoop(void* parameter);
void LogPrint(const char* str);
void LogPrintln(const char* str);
void LogPrint(float flt);
void LogPrintln(float flt);
void LogPrint(uint32_t i);
void LogPrintln(uint32_t i);
void LogPrint(int32_t i);
void LogPrintln(int32_t i);

#endif
