#include <Michom.h>
#include <Termometrs.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 10

char* id = "termometr_okno";
const char* type = Termometr;
double VersionTermometr = 1.63;
/////////настройки//////////////

Michome michome(id, type, VersionTermometr);
TermometrModules terms(&michome);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer[MaxTermometers];

CreateMichome;

void setup ( void ) {
  
  terms.GetTempHandler(GetTemp);

  terms.GetTermomersInfoHandler([](uint8_t idTermometrs) 
  {
    String tmp = (String)"Тип термодатчика: DS18B20" + "<br />";
    tmp += "Паразитное питание на линии: " + String((sensors.isParasitePowerMode() ? "включено" : "выключено")) + "<br />";
    tmp += "Номер датчика: " + String(idTermometrs) + "<br />";
    tmp += "Адрес датчика: " + printAddress(insideThermometer[idTermometrs]) + "<br />";
    tmp += "Разрешение измерения: " + String(sensors.getResolution(insideThermometer[idTermometrs])) + " бит" + "<br />";
    return tmp;  
  });

  terms.GetResetTempHandler([](uint8_t idTermometrs) 
  {
    return true;  
  });

  terms.GetInitTempHandler([](uint8_t idTermometrs) 
  {   
    if(!sensors.getAddress(insideThermometer[idTermometrs], idTermometrs)) {return false;};
    sensors.setResolution(insideThermometer[idTermometrs], 12);  
  });

  michome.SetRefreshData([](){terms.SendData();});
 
  michome.init(false);
  if(michome.IsSaveMode) return;

  sensors.begin();
  uint8_t termCount = sensors.getDeviceCount();
  terms.SetCountTermometrs(termCount);
  
  terms.init();
  
  michome.Refresh();
}

void loop ( void ) {
  michome.running();
  
  if(michome.IsSaveMode) return;
  terms.running();
  
  if(michome.GetSettingRead()){
    terms.ChangeTime(michome.GetSetting("update").toInt());
  } 
}

float GetTemp(uint8_t idTermometrs){
  sensors.requestTemperatures();
  return sensors.getTempC(insideThermometer[idTermometrs]);
}

String printAddress(DeviceAddress deviceAddress)
{
  String tmp = "";
  for (uint8_t i = 0; i < 8; i++)
    tmp += String(deviceAddress[i], HEX);
  return tmp;
}
