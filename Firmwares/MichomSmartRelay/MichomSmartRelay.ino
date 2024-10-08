#include <Michom.h>
#include <LightModules.h>
#include <TimerLightModule.h>
#include <GyverButton.h>

#define ButtonPin 12

#pragma message "Не забываем выбирать праввильную платформу"

char* id = "SmartRelay";
const char* type = StudioLight; //стандартный тип модуля освещения
double Version = 1.27;
/////////настройки//////////////

RTOS rtos(604000); //время опроса до сервера

Michome michome(id, type, Version); //инициальзация модуля Michome
LightModules lm (&michome); //Инициализация модуля освещения
TimerLightModule tlm(&lm); //Инициализация подсистемы точного времени

ESP8266WebServer& server1 = michome.GetServer(); //Получение объекта веб-сервера

GButton butt1(ButtonPin); //Класс кнопки
byte clicks = 0; //Количетво нажатий

CreateMichome;

void setup ( void ) {  
  lm.AddPin({14, Relay}); //Дабавить пин 14 с типом Relay 
  
  lm.TelnetEnable = true; //Включена поддержка telnet запросов
  lm.SaveState = true; //Включено сохранение статуса выводов при перезапуске
  lm.init(); //Инициализация модуля освещения
  tlm.init(); //Инициализация подсистемы времени
  michome.init(true); //Инициализация модуля Michome
  michome.TimeoutConnection = LightModuleTimeoutConnection; //Таймаут соединения до шлюза
  butt1.setClickTimeout(200);

  Module.addButton(0, ButtonPin, ButtonUDP(michome));

  rtos.Stop();
}

void loop ( void ) {
  michome.running(); //Цикличная функция работы
  lm.running(); //Цикличная функция работы
  tlm.running(); //Цикличная функция работы

  if (michome.GetSettingRead()) {
    rtos.ChangeTime(michome.GetSettingToInt("update", 604000));
    butt1.setClickTimeout(michome.GetSettingToInt("clicktimeout", 200));
    if (michome.GetSetting("logging") == "1")
      rtos.Start();
    else
      rtos.Stop();
  }

  if (rtos.IsTick()) {
    michome.SendData();
  } 

  butt1.tick();

  if (butt1.hasClicks()){clicks = butt1.getClicks(); michome.PortPrint("Clicks="+String(clicks), true);}
  else clicks = 0;

  if (clicks > 1) {
    michome.GetUDP().SendTrigger(michome.GetModule(0)+"_Button", String(clicks));
    michome.SendData(michome.ParseJson("get_button_press", String(ButtonPin)+"="+String(clicks)));
  }
  else if (clicks == 1) {
    lm.ExternalReverse(0);
    michome.GetUDP().SendTrigger(michome.GetModule(0)+"_Button", String(clicks));
    michome.GetUDP().SendTrigger(michome.GetModule(0)+"_Button1", String(lm.GetBrightness(0)));
  }
}
