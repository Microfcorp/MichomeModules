#include <Michom.h>
#include <Termometrs.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 10

char* id = "termometr_okno";
const char* type = Termometr;
double VersionTermometr = 1.62;
/////////настройки//////////////

Michome michome(id, type, VersionTermometr);
TermometrModules terms(&michome);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;

void setup ( void ) {
  
  terms.GetTempHandler(GetTemp);

  terms.GetTermomersInfoHandler([]() 
  {
    String tmp = (String)"Тип термодатчика: DS18B20" + "<br />";
    tmp += "Паразитное питание: " + String((sensors.isParasitePowerMode() ? "включено" : "выключено")) + "<br />";
    tmp += "Разрешение измерения: " + String(sensors.getResolution(insideThermometer)) + " бит" + "<br />";
    return tmp;  
  });

  terms.GetResetTempHandler([]() 
  {
    return true;  
  });

  terms.GetInitTempHandler([]() 
  {
    sensors.begin();
    if (!sensors.getAddress(insideThermometer, 0)) { michome.PortPrintln("Unable to find address for Device 0"); return false;}
    sensors.setResolution(insideThermometer, 11);
    return true;  
  });

  michome.SetRefreshData([](){terms.SendData();});
 
  michome.init(false);

  if(michome.IsSaveMode) return;
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

float GetTemp(){
  sensors.requestTemperatures();
  return sensors.getTempC(insideThermometer);
}
