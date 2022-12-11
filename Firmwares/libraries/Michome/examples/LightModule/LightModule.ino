#include <Michom.h>
#include <LightModules.h>

char* id = "LightModuleExample"; //Стандартный ID модуля
const char* type = StudioLight; //Тип модуля
double VersionUploader = 1.1; //Версия ПО модуля
/////////настройки//////////////

RTOS rtos(RTOS10M); //Таймер отправки сообщения на шлюз
Michome michome(id, type, VersionUploader); //Создание модуля Michome
LightModules lm (&michome); //Создание модуля освещения

CreateMichome;

void setup ( void ) { 
  lm.AddPin({4, Relay}); //Дабавить пин 4 с типом Реле 
  lm.AddPin({5, PWM}); //Дабавить пин 5 с типом PWM 
  
  lm.TelnetEnable = true; //Включена поддержка telnet запросов
  lm.SaveState = true; //Включено сохранение статуса выводов при перезапуске
  lm.init(); //Инициализация модуля освещения
  michome.TimeoutConnection = LightModuleTimeoutConnection; //Таймаут соединения до шлюза
  michome.init(true); //Инициализация модуля Michome
}

void loop ( void ) {
  michome.running(); //Цикличная функция работы
  if(michome.IsSaveMode) return; //Сброс инициализации термометра при safe mode
  lm.running(); //Цикличная функция работы

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
