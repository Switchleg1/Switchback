#include "sensors.h"
#include "logs.h"
#include "config.h"
#include "obd2.h"
#include "engine.h"
#include <Adafruit_MCP4728.h>
#include <SPI.h>
#include <Mcp320x.h>

uint16_t sensorTask;
uint16_t sensorCount;
volatile uint16_t writeSensorSpeed;
volatile SENSOR_T Sensors[SENSOR_COUNT];
Adafruit_MCP4728 DAC;
MCP3204 ADC(ADC_VREF, ADC_CS_PIN);

//Timers
hw_timer_t*  DACTimer;

//Semaphores
SemaphoreHandle_t SensorReadSemaphore;
SemaphoreHandle_t SensorTaskSemaphore;

void IRAM_ATTR onDACTimer()
{
  xSemaphoreGiveFromISR(SensorTaskSemaphore, NULL);
}

void SensorsBegin()
{
  LogPrint("Sensors setup: ");

  //Reset EEPROM Settings
#ifdef EEPROM_RESET
  EEPROMWriteByte(EEPROM_PRINT_SPEED, false);
#endif

  //Read EEPROM Settings
  writeSensorSpeed = EEPROMReadByte(EEPROM_PRINT_SPEED);

  //Variables for sensor benchmark
  sensorTask = 0;
  sensorCount = 0;

  //clear sensor data
  for(int i=0; i<SENSOR_COUNT; i++)
  {
    Sensors[i].value = 0;
    Sensors[i].offset = 0;
    Sensors[i].raw = 0;
  }

  //Try to initialize DAC
  if(!DAC.begin())
  {
    LogPrint("MCP4728 failed, ");
  }

  // set I2C clock speed
  Wire.setClock(DAC_CLK);
  
  //clear dac
  DAC.setChannelValue(MCP4728_CHANNEL_A, 0);
  DAC.setChannelValue(MCP4728_CHANNEL_B, 0);
  DAC.setChannelValue(MCP4728_CHANNEL_C, 0);
  DAC.setChannelValue(MCP4728_CHANNEL_D, 0);

  //setup o2 ADC
  //analogSetAttenuation(ADC_0db);
  //adcAttachPin(O2_ADC_PIN);
  
  //setup internal dac for O2 sensor
  dacWrite(DAC1_PIN, 0);

  // configure PIN mode
  pinMode(ADC_CS_PIN, OUTPUT);

  // set initial PIN state
  digitalWrite(ADC_CS_PIN, HIGH);

  // initialize SPI interface for MCP3204
  SPISettings settings(ADC_CLK, MSBFIRST, SPI_MODE0);
  SPI.begin();
  SPI.beginTransaction(settings);

  //Init Semaphore for synching read/writes and data reading
  SensorReadSemaphore = xSemaphoreCreateBinary();
  SensorTaskSemaphore = xSemaphoreCreateBinary();

  //Enable DAC Timer to ensure correct VDD and to check speed set for 1s
  DACTimer = timerBegin(0, 16000, true);
  timerAttachInterrupt(DACTimer, &onDACTimer, true);
  timerAlarmWrite(DACTimer, 5000, true);
  timerAlarmEnable(DACTimer);
  
  LogPrintln("complete.");
}

void SensorsReadLoop(void* parameter)
{
  for(;;)
  {
#ifdef DEBUG_TASK
    LogPrintln("Read task: running.");
#endif
    if(xSemaphoreTake(SensorReadSemaphore, 1) == pdPASS)
    {
      Sensors[0].raw = ADC.read(MCP3204::Channel::SINGLE_3);
      yield(); 
      Sensors[1].raw = ADC.read(MCP3204::Channel::SINGLE_2);
      yield(); 
      Sensors[2].raw = ADC.read(MCP3204::Channel::SINGLE_1);
    }
    yield(); 
  }
}

void SensorsWriteLoop(void* parameter)
{
  for(;;)
  {
#ifdef DEBUG_TASK
    LogPrintln("Write task: running.");
#endif

    Sensors[0].value = Sensors[0].raw + Sensors[0].offset;
    Sensors[1].value = Sensors[1].raw + Sensors[1].offset;
    Sensors[2].value = Sensors[2].raw + Sensors[2].offset;

    xSemaphoreGive(SensorReadSemaphore);
    
    if(Sensors[0].value < 0)
      Sensors[0].value = 0;

    if(Sensors[1].value < 0)
      Sensors[1].value = 0;

    if(Sensors[2].value < 0)
      Sensors[2].value = 0;
    
    if(xSemaphoreTake(SensorTaskSemaphore, 0) == pdPASS)
    {
      //Alternate between setting VDD and writing dac speed
      if(++sensorTask % 2)
      { //update DAC reference to VDD
        DAC.setChannelValue(MCP4728_CHANNEL_A, Sensors[0].value);
        DAC.setChannelValue(MCP4728_CHANNEL_B, Sensors[1].value);
        DAC.setChannelValue(MCP4728_CHANNEL_C, Sensors[2].value);
      } else
      { //write DAC speed
        if(writeSensorSpeed)
        {
          LogPrint("DAC: ");
          LogPrintln(sensorCount/2);
        }
        DAC.fastWrite(Sensors[0].value, Sensors[1].value, Sensors[2].value, 0);
        sensorCount = 0;  
      }
    } else
    {
      DAC.fastWrite(Sensors[0].value, Sensors[1].value, Sensors[2].value, 0);
      sensorCount++;
    }

    yield(); 
  } 
}

void SensorsWriteSpeed(uint16_t active)
{
  if(active)
    sensorCount = 0;
    
  writeSensorSpeed = active;

  EEPROMWriteByte(EEPROM_PRINT_SPEED, writeSensorSpeed);
}

uint16_t SensorsWriteSpeed()
{
  return writeSensorSpeed;
}
