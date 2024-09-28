#include <Michom.h>
#include <Termometrs.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 10 //Шина OneWire

const char* id = "termometr_example"; //Стандартный ID модуля
const char* type = Termometr; //Тип модуля
double VersionTermometr = 1.1; //Версия ПО модуля
/////////настройки//////////////

Michome michome(id, type, VersionTermometr); //Создание модуля Michome
TermometrModules terms(&michome); //Создание класса TermometrModules

OneWire oneWire(ONE_WIRE_BUS); //Создание класса OneWire
DallasTemperature sensors(&oneWire); //Создание класса DallasTemperature
DeviceAddress insideThermometer; //Создание класса DeviceAddress

CreateMichome;

void setup ( void ) {
  
  terms.GetTempHandler(GetTemp); //Обработчик получения температуры (float)

  terms.GetTermomersInfoHandler([]() //Обработчик получения информации о термометре (String)
  {
    String tmp = (String)"Тип термодатчика: DS18B20" + "<br />";
    tmp += "Паразитное питание: " + String((sensors.isParasitePowerMode() ? "включено" : "выключено")) + "<br />";
    tmp += "Разрешение измерения: " + String(sensors.getResolution(insideThermometer)) + " бит" + "<br />";
    return tmp;  
  });

  terms.GetResetTempHandler([]() //Обработчик сброса термометра (bool)
  {
    return true;  
  });

  terms.GetInitTempHandler([]() //Обработчик инициализации термометра (bool)
  {
    sensors.begin();
    if (!sensors.getAddress(insideThermometer, 0)) { michome.PortPrintln("Unable to find address for Device 0"); return false;}
    sensors.setResolution(insideThermometer, 11);
    return true;  
  });

  michome.SetRefreshData([](){terms.SendData();}); //Обработчик отправки температуры на шлюз
 
  michome.init(false); //Инициализация Michome
  if(michome.IsSaveMode) return; //Сброс инициализации термометра при safe mode
  terms.init(); //Инициализация термометра
  michome.Refresh(); //Отправка данных на шлюз
}

void loop ( void ) {
  michome.running(); //Цикличная функция работы 
  if(michome.IsSaveMode) return; //Сброс термометра при safe mode
  terms.running();//Цикличная функция работы 
  
  if(michome.GetSettingRead()){
    terms.ChangeTime(michome.GetSetting("update").toInt()); //Чтение настроек
  } 
}

float GetTemp(){ //Получение температуры
  sensors.requestTemperatures();
  return sensors.getTempC(insideThermometer);
}