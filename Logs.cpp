#include "logs.h"
#include "config.h"
#include "obd2.h"
#include "sensors.h"
#include "engine.h"
#include "tables.h"

#ifdef DEBUG

#ifdef DEBUG_BT
#include "BluetoothSerial.h"
#endif

typedef struct
{
	char		data[MAX_STRING];
	uint16_t	length;
	uint16_t	readable;
} RXBuffer_t;

//Serial buffers
#ifdef DEBUG_BT
BluetoothSerial SerialBT;
RXBuffer_t      BTBuffers[RXBUFFER_COUNT];
uint16_t		BTBufferWrite = 0;
uint16_t		BTBufferRead = 0;
#endif

#ifdef DEBUG_SERIAL
RXBuffer_t  SerialBuffers[RXBUFFER_COUNT];
uint16_t	SerialBufferWrite = 0;
uint16_t	SerialBufferRead = 0;
#endif

//other
#if DEBUG_BT
uint16_t	AdminAccess = false;
#else
uint16_t  AdminAccess = true;
#endif

#ifdef DEBUG_BT
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
#endif

#ifdef DEBUG_SERIAL
void ReadSerial(char value)
{
  RXBuffer_t* pBuffer = &SerialBuffers[SerialBufferWrite];
  pBuffer->data[pBuffer->length] = value;

  if(pBuffer->data[pBuffer->length] == 10)
  {
      pBuffer->data[pBuffer->length+1] = 0;
      pBuffer->readable = true;
      if(++SerialBufferWrite >= RXBUFFER_COUNT)
        SerialBufferWrite -= RXBUFFER_COUNT;
      SerialBuffers[SerialBufferWrite].length = 0;
      SerialBuffers[SerialBufferWrite].readable = false;
  } 
  
  if(pBuffer->length < MAX_STRING-1)
      pBuffer->length++;
}
#endif

#ifdef DEBUG_BT
void ReadSerialBT(char value)
{
  RXBuffer_t* pBuffer = &BTBuffers[BTBufferWrite];
  pBuffer->data[pBuffer->length] = value;

  if(pBuffer->data[pBuffer->length] == 10)
  {
      pBuffer->data[pBuffer->length+1] = 0;
      pBuffer->readable = true;
      if(++BTBufferWrite >= RXBUFFER_COUNT)
        BTBufferWrite -= RXBUFFER_COUNT;
      BTBuffers[BTBufferWrite].length = 0;
      BTBuffers[BTBufferWrite].readable = false;
  } 
  
  if(pBuffer->length < MAX_STRING-1)
      pBuffer->length++;
}

void BTcallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
  if(event == ESP_SPP_SRV_OPEN_EVT)
  {
    AdminAccess = false;
    LogPrintln("BT Client: Connected.");
  }
 
  if(event == ESP_SPP_CLOSE_EVT)
  {
    AdminAccess = false;
    LogPrintln("BT Client: Disconnected.");
  }
}
#endif

void ParseBuffer(RXBuffer_t* pBuffer)
{
  if(!pBuffer || !pBuffer->data)
    return;

  char paramCount = 0;
  char params[5][MAX_STRING];
    
  switch(pBuffer->data[0])
  {
    case 'G':
      if(AdminAccess) 
      {
        analogSetAttenuation(ADC_0db);
        adcAttachPin(O2_ADC_PIN);
        LogPrint("O2 Value: ");
        LogPrintln(analogRead(O2_ADC_PIN));
      } else 
      {
        LogPrintln("No access.");
      }
      break;
    case 'O':
      if(AdminAccess) 
      {
        sscanf(pBuffer->data, "O %s", params[0]);
        EngineSetDefaultO2Value(atoi(params[0]));
        
      } else 
      {
        LogPrintln("No access.");
      }
      break;
    case 'P':
      sscanf(pBuffer->data, "P %s", params[0]);
      if(params[0] == 0)
      {
        LogPrint("Admin Access: ");
        LogPrintln(AdminAccess);
      } else if(!strcmp(params[0], ADMIN_PASSWORD))
      {
        LogPrintln("Admin Access: Granted.");
        AdminAccess = true;
      } else
      {
        LogPrintln("Admin Access: Incorrect Password.");
        AdminAccess = false;
      }
      break;
    case 'R':
      if(AdminAccess) 
      {
        if(OBD2PrintPIDS())
        {
          OBD2PrintPIDS(false);
        } else
        {
          OBD2PrintPIDS(true); 
        }
      } else 
      {
        LogPrintln("No access.");
      }
      break;
    case 'T':
      if(AdminAccess) 
      {
        paramCount = sscanf(pBuffer->data, "T %s %s %s %s", params[0], params[1], params[2], params[3]);
        if(params[0][0] == 'R' && paramCount == 3 && strlen(params[0]) == 1 && strlen(params[1]) == 1 && strlen(params[2]) == 1)
        {
          LogPrint("Set table value: ");
          LogPrintln(GetEngineTableValue(atoi(params[1]), atoi(params[2])));
        } else if(params[0][0] == 'W' && paramCount == 4 && strlen(params[0]) == 1 && strlen(params[1]) == 1 && strlen(params[2]) == 1)
        {
          SetEngineTableValue(atoi(params[1]), atof(params[3]), atoi(params[2]), true);
          LogPrint("Get table value: ");
          LogPrintln(float(atof(params[3])));
        }
      } else 
      {
        LogPrintln("No access.");
      }
      break;
    case 'W':
      if(AdminAccess) 
      {
        if(SensorsWriteSpeed())
        {
          SensorsWriteSpeed(false);
        } else
        {
          SensorsWriteSpeed(true);
        }
      } else 
      {
        LogPrintln("No access.");
      }
      break;
  }
}

#endif

void LogBegin()
{
#ifdef DEBUG
#ifdef DEBUG_SERIAL
  //setup serial port
  Serial.begin(SERIAL_BAUD);
#endif

#ifdef DEBUG_BT
  //setup bt serial port
  SerialBT.register_callback(BTcallback);
  SerialBT.begin(FIRMWARE_NAME);
#endif
#endif
}

void LogLoop(void* parameter)
{
  LogPrintln("Log task: started.");
  for(;;)
  {
#ifdef DEBUG_TASK
    LogPrintln("Log task: running.");
#endif

#ifdef DEBUG_SERIAL
    //Check serial port
    while(Serial.available())
      ReadSerial(Serial.read());
  
    //Check SerialRead buffers for valid data
    while(SerialBufferRead != SerialBufferWrite && SerialBuffers[SerialBufferRead].readable)
    {
        ParseBuffer(&SerialBuffers[SerialBufferRead]);
  
        SerialBuffers[SerialBufferRead].readable = false;
        if(++SerialBufferRead >= RXBUFFER_COUNT)
          SerialBufferRead -= RXBUFFER_COUNT;
    }
#endif

#ifdef DEBUG_BT
    //Check serial port
    while(SerialBT.available())
      ReadSerialBT(SerialBT.read());
  
    //Check BTSerialRead buffers for valid data
    while(BTBufferRead != BTBufferWrite && BTBuffers[BTBufferRead].readable)
    {
        ParseBuffer(&BTBuffers[BTBufferRead]);
  
        BTBuffers[BTBufferRead].readable = false;
        if(++BTBufferRead >= RXBUFFER_COUNT)
          BTBufferRead -= RXBUFFER_COUNT;
    }
#endif

    vTaskDelay(10);
  }
}

void LogPrint(const char* str)
{
#ifdef DEBUG
#ifdef DEBUG_SERIAL
  Serial.print(str);
#endif

#ifdef DEBUG_BT
  SerialBT.print(str);
#endif
#endif
}

void LogPrintln(const char* str)
{
#ifdef DEBUG
#ifdef DEBUG_SERIAL
  Serial.println(str);
#endif
#ifdef DEBUG_BT
  SerialBT.println(str);
#endif
#endif
}

void LogPrint(float flt)
{
#ifdef DEBUG
#ifdef DEBUG_SERIAL
  Serial.print(flt);
#endif
#ifdef DEBUG_BT
  SerialBT.print(flt);
#endif
#endif
}

void LogPrintln(float flt)
{
#ifdef DEBUG
#ifdef DEBUG_SERIAL
  Serial.println(flt);
#endif
#ifdef DEBUG_BT
  SerialBT.println(flt);
#endif
#endif
}

void LogPrint(uint32_t i)
{
#ifdef DEBUG
#ifdef DEBUG_SERIAL
  Serial.print(i);
#endif
#ifdef DEBUG_BT
  SerialBT.print(i);
#endif
#endif
}

void LogPrintln(uint32_t i)
{
#ifdef DEBUG
#ifdef DEBUG_SERIAL
  Serial.println(i);
#endif
#ifdef DEBUG_BT
  SerialBT.println(i);
#endif
#endif
}

void LogPrint(int32_t i)
{
#ifdef DEBUG
#ifdef DEBUG_SERIAL
  Serial.print(i);
#endif
#ifdef DEBUG_BT
  SerialBT.print(i);
#endif
#endif
}

void LogPrintln(int32_t i)
{
#ifdef DEBUG
#ifdef DEBUG_SERIAL
  Serial.println(i);
#endif
#ifdef DEBUG_BT
  SerialBT.println(i);
#endif
#endif
}
