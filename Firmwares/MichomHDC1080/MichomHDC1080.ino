#define FIRModuleVersion 1.3

#include <Michom.h>
#include <ClosedCube_HDC1080.h>
#include <LightModules.h>
#include <TimerLightModule.h>
#include <MichomUDP.h>

ClosedCube_HDC1080 hdc1080ax; 

const char* id = "Garland_Controller";
const char* type = "hdc1080mx"; //GenerateTypes(new String[2]{String(StudioLight), String(HDC1080md)}, 2);
/////////настройки//////////////

RTOS rtos(604000);

Michome michome(id, type);

ESP8266WebServer& server1 = michome.GetServer();
LightModules lm (&michome);
TimerLightModule tlm(&lm);
MichomeUDP MUDP(&michome); //Созлдание класса UDP кнотроллера

void setup ( void ) {
  lm.AddPin({10, Relay});
  
  lm.TelnetEnable = true;
  lm.SaveState = true; //Включено сохранение статуса выводов при перезапуске
  lm.init();
  tlm.init();
  michome.init(false);
  michome.TimeoutConnection = LightModuleTimeoutConnection;
  
  hdc1080ax.begin(0x40);

  MUDP.lightModules = &lm; //Ссылка на объект модуля освещения
  MUDP.timerLightModules = &tlm; //Ссылка на объект подсисетмы времени
  //MUDP.EAlarm = true; //Включено событие EAlarm
  MUDP.init(); //Инициализация модуля UDP
  
  server1.on("/refresh", [](){ 
    server1.send(200, "text/html", "OK");
    SendData();
  });

  server1.on(MeteoRequest, [](){ 
    String tmp = "<html><head><title>Метеоданные</title><meta charset=\"UTF-8\"></head><body><table><tbody>";
    tmp += "<tr><td>Температура: "+String(hdc1080ax.readTemperature())+"<td></tr>";
    tmp += "<tr><td>Влажность: "+String(hdc1080ax.readHumidity())+"<td></tr>";
    tmp += "</tbody></table></body><html>";
    server1.send(200, "text/html", tmp);
    //SendData();
  });

  server1.on("/gettemper", [](){ 
    server1.send(200, "text/html", String(hdc1080ax.readTemperature()));
  });

  server1.on("/gethumm", [](){ 
    server1.send(200, "text/html", String(hdc1080ax.readHumidity()));
  });
  
  SendData();
}

void loop ( void ) {
  michome.running(); //Цикличная функция работы
  lm.running(); //Цикличная функция работы
  tlm.running(); //Цикличная функция работы
  MUDP.running(); //Цикличная функция работы

  if(michome.GetSettingRead()){
    //rtos.ChangeTime(michome.GetSettingToInt("update"));
  }
  
  if (rtos.IsTick())
    SendData();
}

void SendData(){
  michome.SendData(michome.ParseJson(String(type), String(hdc1080ax.readTemperature())+";"+String(hdc1080ax.readHumidity())));
}
