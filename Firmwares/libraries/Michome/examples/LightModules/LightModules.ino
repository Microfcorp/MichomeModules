#include <Michom.h>
#include <LightModules.h>
#include <TimerLightModule.h>

const char* id = "LightStudio_Elka";
const char* type = StudioLight; //стандартный тип модуля освещения
double VersionUploader = 1.1;
/////////настройки//////////////

RTOS rtos(604000); //время опроса до сервера

Michome michome(id, type, VersionUploader); //инициальзация модуля Michome
LightModules lm (&michome); //Инициализация модуля освещения
TimerLightModule tlm(&lm); //Инициализация подсистемы точного времени

ESP8266WebServer& server1 = michome.GetServer(); //Получение объекта веб-сервера

void setup ( void ) {
  lm.AddPin({4, Relay}); //Дабавить пин 4 с типом Реле 
  lm.AddPin({5, PWM}); //Дабавить пин 5 с типом PWM 
  
  lm.TelnetEnable = true; //Включена поддержка telnet запросов
  lm.SaveState = true; //Включено сохранение статуса выводов при перезапуске
  lm.init(); //Инициализация модуля освещения
  tlm.init(); //Инициализация подсистемы времени
  michome.init(true); //Инициализация модуля Michome
  michome.TimeoutConnection = LightModuleTimeoutConnection; //Таймаут соединения до шлюза
  
  //MUDP.init(); //Инициализация модуля UDP
  
  server1.on("/refresh", [](){ 
    server1.send(200, "text/html", "OK");
    michome.SendData();
  });
}

void loop ( void ) {
  michome.running(); //Цикличная функция работы
  lm.running(); //Цикличная функция работы
  tlm.running(); //Цикличная функция работы
  //MUDP.running(); //Цикличная функция работы

  if (michome.GetSettingRead()) {
    rtos.ChangeTime(michome.GetSettingToInt("update"));
    if (michome.GetSetting("logging") == "1")
      rtos.Start();
    else
      rtos.Stop();
  }

  if (rtos.IsTick()) {
    michome.SendData();
  } 
}
