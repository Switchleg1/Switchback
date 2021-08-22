#include "config.h"
#include "obd2.h"
#include "sensors.h"
#include "tables.h"
#include "engine.h"
#include "logs.h"

//defines
#define PSI             81.7f
#define O2_OFFSET       35
#define MIN_RPM         500

//static variables
uint16_t  printSensorCount;
uint16_t  printSensorPos;
volatile float    engineBoostAverage;
volatile uint16_t engineDefaultO2Value;

void EngineBegin()
{
  LogPrint("Engine setup: ");

  printSensorCount = 0;
  printSensorPos = 0;
  engineBoostAverage = 0;
  engineDefaultO2Value = 0;

#ifdef EEPROM_RESET
  WriteSensorTable(SEN_TABLE_MAP);
  WriteSensorTable(SEN_TABLE_PUT);
  WriteSensorTable(SEN_TABLE_MAPR);
  WriteSensorTable(SEN_TABLE_PUTR);
  WriteEngineTable(ENG_TABLE_BOOST_PEAK);
  WriteEngineTable(ENG_TABLE_BOOST_MULT);
  WriteEngineTable(ENG_TABLE_BOOST_CAP);
  WriteEngineTable(ENG_TABLE_FUEL);
  WriteEngineTable(ENG_TABLE_AFR);
  WriteEngineTable(ENG_TABLE_AFRB);
  WriteEngineTable(ENG_TABLE_IAT);
  WriteEngineTable(ENG_TABLE_COOLANT);
  WriteEngineTable(ENG_TABLE_SPEED);
#endif

  //Read values from EEPROM
  ReadSensorTable(SEN_TABLE_MAP);
  ReadSensorTable(SEN_TABLE_PUT);
  ReadSensorTable(SEN_TABLE_MAPR);
  ReadSensorTable(SEN_TABLE_PUTR);
  ReadEngineTable(ENG_TABLE_BOOST_PEAK);
  ReadEngineTable(ENG_TABLE_BOOST_MULT);
  ReadEngineTable(ENG_TABLE_BOOST_CAP);
  ReadEngineTable(ENG_TABLE_FUEL);
  ReadEngineTable(ENG_TABLE_AFR);
  ReadEngineTable(ENG_TABLE_AFRB);
  ReadEngineTable(ENG_TABLE_IAT);
  ReadEngineTable(ENG_TABLE_COOLANT);
  ReadEngineTable(ENG_TABLE_SPEED);
  
  LogPrintln("complete");
}

void EngineLoop()
{
  if(OBD2EngineStarted())
  { //Obd2 closed loop
#ifndef DEBUG_SENSORS
    //read current obd2 values
    float absolutePressure = PIDValues[PIDS_BAROMETRIC_PRESSURE] / 6.871f;
    float currentRPM = PIDValues[PIDS_ENGINE_RPM];
    float currentIAT = PIDValues[PIDS_INTAKE_TEMPERATURE];
    float currentTiming = PIDValues[PIDS_TIMING_ADVANCE];
    float currentThottle = PIDValues[PIDS_THROTTLE_POSITION];
    float currentCoolant = PIDValues[PIDS_COOLANT_TEMPERATURE];
    float currentSpeed = PIDValues[PIDS_SPEED];
    float currentAFR = PIDValues[PIDS_AFR];
    float currentSTFT = PIDValues[PIDS_STFT];
    float currentLTFT = PIDValues[PIDS_LTFT];
    
    //Make sure OBD2 values are some what legit (10psi to just under sea level)
    if(currentRPM > MIN_RPM && absolutePressure > 10.0 && absolutePressure < 15.5)
    {
      //read current sensor values
      int32_t rawFuel = Sensors[SENSOR_FUEL].raw;
      int32_t rawBoost = Sensors[SENSOR_BOOST].raw;
      int32_t rawManifold = Sensors[SENSOR_MANIFOLD].raw;
      float   psiBoost = FindSensorTableValue(rawBoost, SEN_TABLE_PUT)-absolutePressure;
      float   psiManifold = FindSensorTableValue(rawManifold, SEN_TABLE_MAP)-absolutePressure;
  
      //determine new boost offsets
      float boostMult = FindEngineTableValue(currentRPM, ENG_TABLE_BOOST_MULT);
      float boostOffset = (psiBoost>0)?-psiBoost*boostMult:0;
      float manifoldOffset = (psiManifold>0)?-psiManifold*boostMult:0;
      
      //See if we have reached our boost cap and if so limit offsets
      float boostCap = -FindEngineTableValue(currentRPM, ENG_TABLE_BOOST_CAP);
      if(boostOffset < boostCap)
        boostOffset = boostCap;
 
      if(manifoldOffset < boostCap)
        manifoldOffset = boostCap;

      //Compensate for high intake temperatures
      float iatMult = FindEngineTableValue(currentIAT, ENG_TABLE_IAT);
      boostOffset *= iatMult;
      manifoldOffset *= iatMult;

      //Compensate for low coolant temperatures
      float coolantMult = FindEngineTableValue(currentCoolant, ENG_TABLE_COOLANT);
      boostOffset *= coolantMult;
      manifoldOffset *= coolantMult;

      //Compensate for low speed traction
      float speedMult = FindEngineTableValue(currentSpeed, ENG_TABLE_SPEED);
      boostOffset *= speedMult;
      manifoldOffset *= speedMult;
  
      //Make sure we dont go above peak boost
      engineBoostAverage = (engineBoostAverage*0.75) + (psiManifold*0.25);
      if(engineBoostAverage < 0)
      {
        engineBoostAverage = 0;
      } else {
        float boostMax = FindEngineTableValue(currentRPM, ENG_TABLE_BOOST_PEAK);
        if(engineBoostAverage > boostMax)
        {
          boostOffset += engineBoostAverage - boostMax;
          manifoldOffset += engineBoostAverage - boostMax;
        }
      }

      //never have positive boost/manifold offsets
      if(boostOffset > 0)
        boostOffset = 0;

      if(manifoldOffset > 0)
        manifoldOffset = 0;

      //determine new fuel offsets
      float o2Offset = 0;
      float fuelOffset = 0;
      if(manifoldOffset<0)
      {
        o2Offset = FindEngineTableValue(psiManifold, ENG_TABLE_AFRB)*FindEngineTableValue(currentRPM, ENG_TABLE_AFR)*130.0;
        fuelOffset = (float)rawFuel*FindEngineTableValue(currentRPM, ENG_TABLE_FUEL)*((manifoldOffset/(psiManifold+absolutePressure))-(o2Offset*0.00769));
  
        //Correct for O2 variation
        float clampedAFR = currentAFR;
        if(clampedAFR > 1.3) clampedAFR = 1.3;
          else if(clampedAFR < 0.7) clampedAFR = 0.7;
        fuelOffset = fuelOffset * (0.4+(0.6*clampedAFR));
      }

      //smooth offsets
      boostOffset = (boostOffset * PSI * 0.5) + ((float)Sensors[SENSOR_BOOST].offset * 0.5);
      manifoldOffset = (manifoldOffset * PSI * 0.5) + ((float)Sensors[SENSOR_MANIFOLD].offset * 0.5);
      fuelOffset = (fuelOffset * 0.5) + ((float)Sensors[SENSOR_FUEL].offset * 0.5);
      o2Offset = (o2Offset * 0.5) + ((float)Sensors[SENSOR_O2].offset * 0.5);

      //write new offsets
      Sensors[SENSOR_BOOST].offset = boostOffset;
      Sensors[SENSOR_MANIFOLD].offset = manifoldOffset;
      Sensors[SENSOR_FUEL].offset = fuelOffset;
      Sensors[SENSOR_O2].offset = o2Offset;

      //write calculated O2 value
      if(o2Offset)
      {
        float o2OffsetPercent = o2Offset*0.77;
        float o2ActualOffset = o2OffsetPercent*0.01;
        
        if(currentAFR > 1.0)
        {
          Sensors[SENSOR_O2].raw = (currentAFR*100)-o2OffsetPercent;
        } else if(currentAFR < 1.0-o2ActualOffset)
        {
          Sensors[SENSOR_O2].raw = (currentAFR*100)-(o2ActualOffset*50);
        } else
        {
          Sensors[SENSOR_O2].raw = (100.0-o2OffsetPercent)-((1.0-currentAFR)*50);
        }
      } else
      {
        Sensors[SENSOR_O2].raw = currentAFR*100;
      }
      
  
      //only update dac if value has changed
      if(Sensors[SENSOR_O2].value != Sensors[SENSOR_O2].offset)
      {
        Sensors[SENSOR_O2].value = Sensors[SENSOR_O2].offset;
        dacWrite(DAC1_PIN, Sensors[SENSOR_O2].offset?Sensors[SENSOR_O2].offset+O2_OFFSET:0);
      }
    } else
    {
      //clear boost average
      engineBoostAverage = 0;
      
      //clear sensor offsets
      Sensors[SENSOR_BOOST].offset = 0;
      Sensors[SENSOR_MANIFOLD].offset = 0;
      Sensors[SENSOR_FUEL].offset = 0;
      Sensors[SENSOR_O2].offset = 0;

      //only update dac if value has changed
      if(Sensors[SENSOR_O2].value != Sensors[SENSOR_O2].offset)
      {
        Sensors[SENSOR_O2].value = Sensors[SENSOR_O2].offset;
        dacWrite(DAC1_PIN, Sensors[SENSOR_O2].offset?Sensors[SENSOR_O2].offset+O2_OFFSET:0);
      }
    }
#endif
  } else
  { //Open loop mode (debug sensors only)
    //clear sensor offsets
    Sensors[SENSOR_BOOST].offset = 0;
    Sensors[SENSOR_MANIFOLD].offset = 0;
    Sensors[SENSOR_FUEL].offset = 0;
    Sensors[SENSOR_O2].offset = engineDefaultO2Value;
    
    //only update dac if value has changed
    if(Sensors[SENSOR_O2].value != Sensors[SENSOR_O2].offset)
    {
      Sensors[SENSOR_O2].value = Sensors[SENSOR_O2].offset;
      dacWrite(DAC1_PIN, Sensors[SENSOR_O2].offset);
    }

    if(OBD2PrintPIDS() && OBD2Active() != 2)
    {
      if(++printSensorCount % 50 == 0)
      {
        if(printSensorPos)
          LogPrint(",");
          
        switch(printSensorPos)
        {
          case 0:
            LogPrint(Sensors[0].raw);
            break;
          case 1:
            LogPrint(Sensors[1].raw);
            break;
          case 2:
            LogPrintln(Sensors[2].raw);
            break;
        }
          if(++printSensorPos >= 3)
            printSensorPos = 0;
      }
    } else
    {
      printSensorPos = 0;
    }
  }
}

void EngineSetDefaultO2Value(uint16_t v)
{
  if(v > 100)
    v = 100;
    
  engineDefaultO2Value = v;
}

float EngineBoostAverage()
{
  return engineBoostAverage;
}
