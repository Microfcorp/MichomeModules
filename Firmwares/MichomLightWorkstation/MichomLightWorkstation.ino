#include <Michom.h>
#include <LightModules.h>
#include <TimerLightModule.h>
#include <GyverButton.h>

#define ButtonPin 10

char* id = "LightWorkstation";
const char* type = StudioLight; //стандартный тип модуля освещения
double Version = 1.23;
/////////настройки//////////////

RTOS rtos(604000); //время опроса до сервера

Michome michome(id, type, Version); //инициальзация модуля Michome
LightModules lm (&michome); //Инициализация модуля освещения
TimerLightModule tlm(&lm); //Инициализация подсистемы точного времени

ESP8266WebServer& server1 = michome.GetServer(); //Получение объекта веб-сервера

GButton butt1(ButtonPin); //Класс кнопки
byte clicks = 0; //Количетво нажатий

void setup ( void ) { 
  lm.AddPin({14, PWM}); //Дабавить пин 14 с типом PWM 
  lm.AddPin({5, PWM}); //Дабавить пин 5 с типом PWM
  
  lm.TelnetEnable = true; //Включена поддержка telnet запросов
  lm.SaveState = true; //Включено сохранение статуса выводов при перезапуске
  lm.init(); //Инициализация модуля освещения
  tlm.init(); //Инициализация подсистемы времени
  michome.init(true); //Инициализация модуля Michome
  michome.TimeoutConnection = LightModuleTimeoutConnection; //Таймаут соединения до шлюза
  butt1.setClickTimeout(200);

  Module.addButton(0, ButtonPin, ButtonUDP(michome));
}

void loop ( void ) {
  michome.running(); //Цикличная функция работы
  if(michome.IsSaveMode) return;
  
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

  if (butt1.hasClicks()){clicks = butt1.getClicks(); michome.PortPrintln("Clicks="+String(clicks));}
  else clicks = 0;

  if (clicks == 3) {
    lm.ExternalSetLightID(0, MaximumBrightnes/2);
  }
  else if (clicks == 2) {
    lm.StopAllFade();
    lm.ExternalSetLightID(0, MinimumBrightnes);
    michome.GetUDP().SendTrigger((String)michome.GetModule(0)+"_Button", "0");
  }
  else if (clicks == 1) {
    lm.ExternalSetLightID(0, MaximumBrightnes);
    michome.GetUDP().SendTrigger((String)michome.GetModule(0)+"_Button", "1");
  }
  if(clicks != 0 && clicks != 1 && clicks != 2)
  {
    michome.GetUDP().SendTrigger((String)michome.GetModule(0)+"_Button", String(clicks));    
    michome.SendData(michome.ParseJson("get_button_press", String(ButtonPin)+"="+String(clicks)));
  }
}
