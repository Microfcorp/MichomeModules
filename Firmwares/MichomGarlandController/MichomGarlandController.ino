#include <Michom.h>
#include <LightModules.h>
#include <TimerLightModule.h>
 
char* DefaultID = "Garland_Controller";
const char* type = StudioLight; //GenerateTypes(new String[2]{String(StudioLight), String(HDC1080md)}, 2);
const double FIRModuleVersion = 1.64;
/////////настройки//////////////

RTOS rtos(604000);

Michome michome(DefaultID, type, FIRModuleVersion);

ESP8266WebServer& server1 = michome.GetServer();
LightModules lm (&michome);
TimerLightModule tlm(&lm);

ADC_MODE(ADC_VCC);

void setup ( void ) {
  lm.AddPin({10, Relay});
  
  lm.TelnetEnable = true;
  lm.SaveState = true; //Включено сохранение статуса выводов при перезапуске
  lm.init();
  tlm.init();
  michome.init(true);
  michome.TimeoutConnection = LightModuleTimeoutConnection;

  //michome.SetRefreshData([](){michome.SendData();});
}

void loop ( void ) {
  michome.running(); //Цикличная функция работы
  lm.running(); //Цикличная функция работы
  tlm.running(); //Цикличная функция работы

  if(michome.GetSettingRead()){
    rtos.ChangeTime(michome.GetSettingToInt("update", 604000));
  }
  
  if (rtos.IsTick())
    michome.SendData();
}
