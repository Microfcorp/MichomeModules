//#include <ArduinoJson.h>
#include <Michom.h>
#include <LightModules.h>
#include <TimerLightModule.h>
#include <MichomUDP.h>

char* id = "LightStudio_Elka";
const char* type = StudioLight;
double VersionUploader = 1.42; //Версия ПО модуля
/////////настройки//////////////

RTOS rtos(RTOS10M);

Michome michome(id, type, VersionUploader);
LightModules lm (&michome);
TimerLightModule tlm(&lm);

ESP8266WebServer& server1 = michome.GetServer();

//const int Keys[] = {9,10,4,5};

CreateMichome;

void setup ( void ) {
  lm.AddPin({4, Relay, true});
  lm.AddPin({5, Relay, true});
  
  lm.TelnetEnable = true;
  lm.SaveState = true;
  lm.init();
  tlm.init();  
  michome.init(true);
  michome.TimeoutConnection = LightModuleTimeoutConnection;  

  /*server1.on("/setlight", []() {
    bool HasChange = false; //можно ли изменить

    //fl.AddLogFile("Is m" + String(server1.hasArg("m")));
    
    if(server1.hasArg("m")) //если не руками
      if(server1.arg("m") == "cron" && AutoRun){ HasChange = true;} //Если крон и можно менять, то ставим флаг на смену
      else if(server1.arg("m") == "cron" && !AutoRun){ HasChange = false;} //Если крон и нельзя менять, то не ставим флаг на смену
      else{ HasChange = true;} //Если не руками и не кроном, то ставим флаг на смену
    else{ HasChange = true;} //Если руками, то ставим флаг на смену

    //fl.AddLogFile("m ravno " + server1.arg("m"));

    if(!server1.hasArg("m") && server1.arg(1).toInt() == 0) AutoRun = true; //Если руками и выключаем, то разрешаем менять
    if(!server1.hasArg("m") && server1.arg(1).toInt() == 1) AutoRun = false; //Если руками и включаем, то запрещаем менять

    if(HasChange){
      digitalWrite(Keys[server1.arg(0).toInt()], server1.arg(1).toInt());
      server1.send(200, "text/html", String(server1.arg(0).toInt()) + " as " + String(server1.arg(1).toInt()));
      EEPROM.write(server1.arg(0).toInt(), (server1.arg(1).toInt() == 1 ? 100 : 1));
    }         
  });*/
  rtos.Stop();
}

//function AutoChangeTime(){postAjax('/timemodule', GET, '', function(d){timemod.innerHTML = d;}); window.setTimeout('AutoChangeTime()',1000);} 
void loop ( void ) {
  michome.running();
  lm.running();
  tlm.running();

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
