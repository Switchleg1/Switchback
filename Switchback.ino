#include "OBD2.h"
#include "config.h"
#include "sensors.h"
#include "engine.h"
#include "logs.h"

//Tasks
#ifdef DEBUG
TaskHandle_t LogTask;
#endif
TaskHandle_t OBD2Task;
TaskHandle_t SensorsReadTask;
TaskHandle_t SensorsWriteTask;

void setup()
{
  //Setup EEPROM
  EEPROMBegin();

  //Setup OBD2
  OBD2Begin();
  
  //Setup the other serial port and clear buffers
  SensorsBegin();

  //Setup the engine loop variables
  EngineBegin();

#ifdef DEBUG
  //setup serial port
  LogBegin();

  //Write init info
  LogPrintln(FIRMWARE_NAME);
  
  //setup UART task on core 1
  xTaskCreatePinnedToCore(
      LogLoop,    /* Function to implement the task */
      "LogTask",  /* Name of the task */
      10000,      /* Stack size in words */
      NULL,       /* Task input parameter */
      0,          /* Priority of the task */
      &LogTask,   /* Task handle. */
      0);         /* Core where the task should run */
#endif

  xTaskCreatePinnedToCore(
      OBD2Loop,           /* Function to implement the task */
      "OBD2Task",         /* Name of the task */
      10000,              /* Stack size in words */
      NULL,               /* Task input parameter */
      0,                  /* Priority of the task */
      &OBD2Task,          /* Task handle. */
      0);                 /* Core where the task should run */
      
  xTaskCreate(
      SensorsReadLoop,    /* Function to implement the task */
      "SensorsReadTask",  /* Name of the task */
      10000,              /* Stack size in words */
      NULL,               /* Task input parameter */
      1,                  /* Priority of the task */
      &SensorsReadTask);  /* Core where the task should run */  
      
  //setup DAC task on core 0
  xTaskCreatePinnedToCore(
      SensorsWriteLoop,    /* Function to implement the task */
      "SensorsWriteTask",  /* Name of the task */
      10000,              /* Stack size in words */
      NULL,               /* Task input parameter */
      1,                  /* Priority of the task */
      &SensorsWriteTask,   /* Task handle. */
      1);                 /* Core where the task should run */  
}

void loop()
{
#ifdef DEBUG_TASK
  LogPrintln("Engine task: running.");
#endif

  EngineLoop();
  
  vTaskDelay(1);
}
