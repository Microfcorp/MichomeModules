#define EnableDS18B20
#define EnableAHT10
#define EnableBMP280
#define EnableHDC1080
#define EnableSHT21

#include <Michom.h>
#include <MeteoStations.h>

#ifndef EnableExternalUnits
  #error This firmware work only with EnableExternalUnits
#endif

#define AHTCount 0
#define MaxDS 0
#define BMPCount 0
#define HDCCount 0
#define SHTCount 0

#define AHT10_ID1 0
#define AHT10_ID2 1
#define HDC1080_ID 2
#define SHT21_ID 3
#define BMP280_ID1 4
#define BMP280_ID2 5
#define DS18_IDSTART 6

#ifdef EnableAHT10
  #include <Wire.h>
  #include <AHT10.h>
  #define AHTCount 2
#endif

#ifdef EnableDS18B20
  #include <OneWire.h>
  #include <DallasTemperature.h>
  #define ONE_WIRE_BUS 10
  #define MaxDS 10
#endif

#ifdef EnableBMP280
  #include <Wire.h>
  #include <Adafruit_BMP280.h>
  #define BMPCount 2
#endif

#ifdef EnableHDC1080
  #include <Wire.h>
  #include "ClosedCube_HDC1080.h"
  #define HDCCount 1
  #define HDCBufferSize 10
#endif

#ifdef EnableSHT21
  #include <Wire.h>
  #include <GyverHTU21D.h>
  #define SHTCount 1
  #define SHTBufferSize 10
#endif

/*
 * Распределение ID следующее:
 * 0, 1 - AHT10
 * 2 - HDC1080
 * 3 - SHT21
 * 4, 5 - BMP280
 * 6 - 15 - DS18B20
 * 
  */

char* id = "meteostation_main";
const char* type = MeteoModule;
double VersionTermometr = 1.42;
/////////настройки//////////////

Michome michome(id, type, VersionTermometr);
MeteoStationModules meteo(&michome);
ExternalUnits& EU = michome.GetExternalUnits();

MeteoStationData GetMeteo(uint8_t idTermometrs);

#ifdef EnableAHT10
  AHT10Class AHT10;
  AHT10Class AHT10_1;
  AHT10Class* AHTS[2] = {&AHT10, &AHT10_1};
  uint8_t countAHT = 0;
#endif

#ifdef EnableHDC1080
  ClosedCube_HDC1080 hdc1080;
  uint8_t countHDC = 0;
  #define HDC1080Address 0x40
  TermometersValue HDCReadingsTemp[HDCBufferSize];
  HummValue HDCReadingsHumm[HDCBufferSize];
  uint8_t countHDCBuffer = 0;
  uint8_t posHDCBuffer = 0;
  bool isHeatHDC = false;
  int secondsForHeat = 0;
  #define secondsForCirle 2
  RTOS HeaterHDC(RTOS1S);
  #define AttemptReadErrorHDC1080 5
  #define MaxHummHeatHDC1080 93
  #define MinTempHeatHDC1080 -10
#endif

#ifdef EnableSHT21
  GyverHTU21D sht21;
  uint8_t countSHT = 0;
  #define SHT21Address 0x40
  TermometersValue SHTReadingsTemp[SHTBufferSize];
  HummValue SHTReadingsHumm[SHTBufferSize];
  uint8_t countSHTBuffer = 0;
  uint8_t posSHTBuffer = 0;
  bool isHeatSHT = false;
  int secondsForHeatSHT = 0;
  #define secondsForCirleSHT 2
  RTOS HeaterSHT(RTOS1S);
#endif

#ifdef EnableBMP280
  Adafruit_BMP280 bmp; // I2C
  Adafruit_BMP280 bmp1; // I2C
  Adafruit_BMP280* BMPS[2] = {&bmp, &bmp1};
  uint8_t countBMP = 0;
#endif

#ifdef EnableDS18B20
  OneWire oneWire(ONE_WIRE_BUS);
  DallasTemperature sensors(&oneWire);
  DeviceAddress insideThermometer[MaxDS];
  uint8_t DsIDs[MaxDS] = {DS18_IDSTART+0, DS18_IDSTART+1, DS18_IDSTART+2, DS18_IDSTART+3, DS18_IDSTART+4, DS18_IDSTART+5, DS18_IDSTART+6, DS18_IDSTART+7, DS18_IDSTART+8, DS18_IDSTART+9};
  uint8_t minds = 0;
  #define MinTempDS18 -100
  #define MaxTempDS18 100
#endif

CreateMichome;

void setup ( void ) {
  
  meteo.GetMeteoHandler(GetMeteo);

  meteo.GetMeteoInfoHandler([](uint8_t idTermometrs) 
  {
    MeteoStationTypeValue data;
    #ifdef EnableAHT10
      if(idTermometrs == AHT10_ID1 || idTermometrs == AHT10_ID2){
          data.temp = "Термометр: AHT10";
          data.humm = "Гигрометр: AHT10";
          data.press = "Барометр: отсутствует";
          data.type = (TypeMeteo)(TypeMeteo::Termo | TypeMeteo::Hummidity);
      }
    #endif
    #ifdef EnableHDC1080
      if(idTermometrs == HDC1080_ID){    
          HDC1080_SerialNumber sernum = hdc1080.readSerialNumber();
          char format[12];
          sprintf(format, "%02X-%04X-%04X", sernum.serialFirst, sernum.serialMid, sernum.serialLast);   
             
          data.temp = "Производитель: 0x" + String(hdc1080.readManufacturerId(), HEX) + (hdc1080.readManufacturerId() == 0x5449 ? " (Texas Instruments)" : "");
          data.humm = "ID устройства: 0x" + String(hdc1080.readDeviceId(), HEX) + " Серийный номер: " + (String)format;
          data.press = "";
          data.type = (TypeMeteo)(TypeMeteo::Termo | TypeMeteo::Hummidity);
      }
    #endif
    #ifdef EnableSHT21
      if(idTermometrs == SHT21_ID){                
          data.temp = "Состояние питания: " + (String)(sht21.powerGood() ? "OK" : "Fail");
          data.humm = "";
          data.press = "";
          data.type = (TypeMeteo)(TypeMeteo::Termo | TypeMeteo::Hummidity);
      }
    #endif
    #ifdef EnableBMP280
      if(idTermometrs == BMP280_ID1 || idTermometrs == BMP280_ID2){          
          data.temp = "Датчик: BMP280";
          data.press = "ID устройства: 0x" + String(idTermometrs == BMP280_ID1 ? bmp.sensorID() : bmp1.sensorID(), HEX);
          data.humm = "";
          data.type = (TypeMeteo)(TypeMeteo::Termo | TypeMeteo::Pressure);
      }
    #endif
    #ifdef EnableDS18B20
      for(byte i = 0; i < MaxDS; i++){
        if(DsIDs[i] == idTermometrs){         
          data.temp = (String)"Тип термодатчика: DS18B20" + "<br />";
          data.temp += "Паразитное питание на линии: " + String((sensors.isParasitePowerMode() ? "включено" : "выключено")) + "<br />";
          data.temp += "Номер датчика: " + String(idTermometrs - DS18_IDSTART) + "<br />";
          data.temp += "Адрес датчика: " + printAddress(insideThermometer[idTermometrs - DS18_IDSTART]) + "<br />";
          data.temp += "Разрешение измерения: " + String(sensors.getResolution(insideThermometer[idTermometrs - DS18_IDSTART])) + " бит" + "<br />";
          data.humm = "";
          data.press = "";
          data.type = (TypeMeteo)(TypeMeteo::Termo);
          return data;
        }
      }
    #endif
    return data;  
  });

  meteo.GetResetMeteoHandler([](uint8_t idTermometrs) 
  {
    return true;  
  });

  meteo.GetInitMeteoHandler([](uint8_t idTermometrs) 
  {
    #ifdef EnableAHT10
      if(idTermometrs == AHT10_ID1 || idTermometrs == AHT10_ID2){
        countAHT = 0;
        if(EU.isWIREFound(eAHT10Address_Low)) countAHT++;
        if(EU.isWIREFound(eAHT10Address_High)) countAHT++;

        if(idTermometrs == AHT10_ID1 && EU.isWIREFound(eAHT10Address_Low)){
          return AHT10.begin(eAHT10Address_Low);
        }
        if(idTermometrs == AHT10_ID2 && EU.isWIREFound(eAHT10Address_High)){
          return AHT10_1.begin(eAHT10Address_High);
        }
        return false;
      }
    #endif

    #ifdef EnableHDC1080
      if(idTermometrs == HDC1080_ID){
        countHDC = 0;
        if(meteo.EnableMeteo[SHT21_ID] && countSHT != 0 && EU.isWIREFound(HDC1080Address)){
          //mchome.AddError(F(__FILE__), F("meteo.GetInitMeteoHandler"), F("SHT21 or HDC1080 one address"));
          return false;
        }  
        
        if(EU.isWIREFound(HDC1080Address)) countHDC++;

        if(EU.isWIREFound(HDC1080Address)){
          hdc1080.begin(HDC1080Address);
          if(hdc1080.readManufacturerId() == 0xFFFF && hdc1080.readDeviceId() == 0xFFFF)
            return false;
          return true;
        }
        return false;
      }
    #endif

    #ifdef EnableSHT21
      if(idTermometrs == SHT21_ID){
        countSHT = 0;
        
        if(meteo.EnableMeteo[HDC1080_ID] && countHDC != 0 && EU.isWIREFound(SHT21Address)){
          //mchome.AddError(F(__FILE__), F("meteo.GetInitMeteoHandler"), F("SHT21 or HDC1080 one address"));
          return false;
        }               
        if(EU.isWIREFound(SHT21Address)) countSHT++;

        if(EU.isWIREFound(SHT21Address)){
          return sht21.begin();
          //return true;
        }
        return false;
      }
    #endif

    #ifdef EnableBMP280
      if(idTermometrs == BMP280_ID1 || idTermometrs == BMP280_ID2){
        countBMP = 0;
        if(EU.isWIREFound(BMP280_ADDRESS)) countBMP++;
        if(EU.isWIREFound(BMP280_ADDRESS_ALT)) countBMP++;

        if(idTermometrs == BMP280_ID1 && EU.isWIREFound(BMP280_ADDRESS)){
          bool tmp = bmp.begin(BMP280_ADDRESS);
          bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
          return tmp;
        }
        if(idTermometrs == BMP280_ID2 && EU.isWIREFound(BMP280_ADDRESS_ALT)){
          bool tmp = bmp1.begin(BMP280_ADDRESS_ALT);
          bmp1.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
          return tmp;
        }
        return false;
      }
    #endif
    
    #ifdef EnableDS18B20
      for(byte i = 0; i < MaxDS; i++){
        if(DsIDs[i] == idTermometrs){
          if(!sensors.getAddress(insideThermometer[idTermometrs - DS18_IDSTART], idTermometrs - DS18_IDSTART)) {return false;};
          sensors.setResolution(insideThermometer[idTermometrs - DS18_IDSTART], 12);
          return true;  
        }
      }
    #endif
    return false;
  });

  #ifdef EnableDS18B20 && 0 //Выключено
    (michome.GetTelnet()).on("minds18","minimum ds18b20 termometrs {minds18;3}", []()
    {
      String data = (michome.GetTelnet()).GetData();
      minds = Split(data, ';', 1).toInt();
     (michome.GetTelnet()).printSucess("Success set minds to " + String(minds));
    });
    (michome.GetTelnet()).on("getminds18","minimum ds18b20 termometrs {getminds18}", []()
    {
      (michome.GetTelnet()).printSucess(String(minds));
    });
  #endif

  #ifdef EnableHDC1080
    (michome.GetTelnet()).on("hdc1080heat","heat up hdc1080 on seconds {hdc1080heat;1}", []()
    {
      if(EU.isWIREFound(HDC1080Address)){
        String data = (michome.GetTelnet()).GetData();
        byte seconds = min((int)10, (int)Split(data, ';', 1).toInt());
        (michome.GetTelnet()).printlnNIA("Start heating hdc1080... ");
        hdc1080.heatUp(seconds);
        (michome.GetTelnet()).printlnNIA("Stoping heating hdc1080");
        (michome.GetTelnet()).println();
      }
      else{
        (michome.GetTelnet()).printlnNIA("hdc1080 not connected");
        (michome.GetTelnet()).println();
      }
    });
  #endif

  michome.SetRefreshData([](){meteo.SendGateway();});
  michome.TimeoutConnection = MeteoStationTimeoutConnection;

  //Wire.setClock(100000);
  //Wire.setClockStretchLimit(1000);
 
  michome.init(false);
  if(michome.IsSaveMode) return;

  uint8_t dsCount = 0;
  #ifdef EnableDS18B20
    sensors.begin();
    dsCount = max(minds, sensors.getDeviceCount());
  #endif

  #ifdef EnableAHT10
    dsCount = dsCount + AHTCount + HDCCount + SHTCount;
  #endif

  #ifdef EnableBMP280
    dsCount = dsCount + BMPCount + HDCCount + SHTCount;
  #endif
  
  meteo.SetCountMeteo(dsCount);
  meteo.TelnetEnable = true;

  meteo.init(); 

  michome.Refresh();
}

void loop ( void ) {
  michome.running();
  
  if(michome.IsSaveMode) return;
  meteo.running();
  
  if(michome.GetSettingRead()){
    meteo.ChangeTime(michome.GetSetting("update").toInt());
  }

  #ifdef EnableHDC1080
    if(HeaterHDC.IsTick() && isHeatHDC){
      Heating();
      if(secondsForHeat < secondsForCirle){
        isHeatHDC = false;
      }
    }
  #endif
}

MeteoStationData GetMeteo(uint8_t idTermometrs){
  MeteoStationData data;
  #ifdef EnableAHT10
    if((idTermometrs == AHT10_ID1 && EU.isWIREFound(eAHT10Address_Low)) || (idTermometrs == AHT10_ID2 && EU.isWIREFound(eAHT10Address_High))){
      data.temp = (*AHTS[idTermometrs]).GetTemperature();
      data.humm = (*AHTS[idTermometrs]).GetHumidity();
      data.press = 0;
      data.type = (TypeMeteo)(TypeMeteo::Termo | TypeMeteo::Hummidity);
      return data;
    }
  #endif
  #ifdef EnableSHT21
    if(idTermometrs == SHT21_ID && EU.isWIREFound(SHT21Address)){
      data.temp = sht21.getTemperatureWait();
      data.humm = sht21.getHumidityWait();
      data.press = 0;
      data.type = (TypeMeteo)(TypeMeteo::Termo | TypeMeteo::Hummidity);
      return data;
    }
  #endif
  #ifdef EnableHDC1080
    if((idTermometrs == HDC1080_ID && EU.isWIREFound(HDC1080Address))){
      if(!isHeatHDC){
        data.temp = hdc1080.readTemperature();
        for(byte i = 0; i < AttemptReadErrorHDC1080 && data.temp >= 120; i++){
            meteo.InitMeteo(idTermometrs);
            delay(50);
            data.temp = hdc1080.readTemperature();
        }
        data.humm = hdc1080.readHumidity();
        data.press = 0;
        AddHDCBuffer(data.temp, data.humm);
        if((AvgHummHDC() >= MaxHummHeatHDC1080 || AvgTempHDC() < MinTempHeatHDC1080) && data.temp < 100){
          secondsForHeat = 30;
          isHeatHDC = true;
        }
      }
      else{
        data.temp = HDCReadingsTemp[posHDCBuffer];
        data.humm = HDCReadingsHumm[posHDCBuffer];
        data.press = 0;
      }
      data.type = (TypeMeteo)(TypeMeteo::Termo | TypeMeteo::Hummidity);
      return data;
    }
  #endif
  #ifdef EnableBMP280
      if((idTermometrs == BMP280_ID1 && EU.isWIREFound(BMP280_ADDRESS)) || (idTermometrs == BMP280_ID2 && EU.isWIREFound(BMP280_ADDRESS_ALT))){
        data.temp = (*BMPS[idTermometrs == BMP280_ID1 ? 0 : 1]).readTemperature();
        data.humm = 0;
        data.press = (*BMPS[idTermometrs == BMP280_ID1 ? 0 : 1]).readPressure()/133.332;
        data.type = (TypeMeteo)(TypeMeteo::Termo | TypeMeteo::Pressure);
        return data;
      }
  #endif
  #ifdef EnableDS18B20
    for(byte i = 0; i < MaxDS; i++){
      if(DsIDs[i] == idTermometrs){
        sensors.requestTemperatures();
        data.temp = sensors.getTempC(insideThermometer[idTermometrs - DS18_IDSTART]);
        if(data.temp < MinTempDS18 || data.temp > MaxTempDS18){
          michome.AddLogFile((String)"ID: " + idTermometrs + " (DS18B20) - Error reading value");
          delay(50);
          data.temp = sensors.getTempC(insideThermometer[idTermometrs - DS18_IDSTART]);
        }
        data.humm = 0;
        data.press = 0;
        data.type = (TypeMeteo)(TypeMeteo::Termo);
        return data;
      }
    }
  #endif
  return data;
}

#ifdef EnableHDC1080
void Heating(){
  if(secondsForHeat < secondsForCirle || !isHeatHDC)
    return;
  hdc1080.heatUp(secondsForCirle);
  secondsForHeat -= secondsForCirle;
}
void AddHDCBuffer(TermometersValue tm, HummValue hu){
  if(isHeatHDC) return;
  if(countHDCBuffer < HDCBufferSize){
    countHDCBuffer++;    
  }
  posHDCBuffer = posHDCBuffer + 1 < countHDCBuffer ? posHDCBuffer + 1 : 0;
  HDCReadingsTemp[posHDCBuffer] = tm;
  HDCReadingsHumm[posHDCBuffer] = hu;
}
TermometersValue AvgTempHDC(){
  TermometersValue tmp = 0;
  for(uint8_t i = 0; i < countHDCBuffer; i++){
    tmp = (tmp + HDCReadingsTemp[i]) / 2;
  }
  return tmp;
}
HummValue AvgHummHDC(){
  HummValue tmp = 0;
  for(uint8_t i = 0; i < countHDCBuffer; i++){
    tmp = (tmp + HDCReadingsHumm[i]) / 2;
  }
  return tmp;
}
#endif

#ifdef EnableDS18B20
String printAddress(DeviceAddress deviceAddress)
{
  String tmp = "";
  for (uint8_t i = 0; i < 8; i++)
    tmp += String(deviceAddress[i], HEX);
  return tmp;
}
#endif
