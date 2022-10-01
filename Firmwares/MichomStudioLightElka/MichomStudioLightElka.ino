//#include <ArduinoJson.h>
#include <Michom.h>
#include <LightModules.h>
#include <TimerLightModule.h>
#include <MichomUDP.h>

const char* id = "LightStudio_Elka";
const char* type = StudioLight;
/////////настройки//////////////

RTOS rtos(604000);

Michome michome(id, type);
LightModules lm (&michome);
TimerLightModule tlm(&lm);

ESP8266WebServer& server1 = michome.GetServer();
MichomeUDP MUDP(&michome);

//const int Keys[] = {9,10,4,5};

void setup ( void ) {
  lm.AddPin({9, Relay});
  lm.AddPin({10, Relay});
  lm.AddPin({4, Relay});
  lm.AddPin({5, PWM});
  
  lm.TelnetEnable = true;
  lm.SaveState = true;
  lm.init();
  tlm.init();  
  michome.init(true);
  michome.TimeoutConnection = LightModuleTimeoutConnection;
  
  MUDP.lightModules = &lm;
  MUDP.timerLightModules = &tlm;
  MUDP.EAlarm = true;
  MUDP.init();
  
  server1.on("/refresh", [](){ 
    server1.send(200, "text/html", "OK");
    michome.SendData();
  });
  

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
}

//function AutoChangeTime(){postAjax('/timemodule', GET, '', function(d){timemod.innerHTML = d;}); window.setTimeout('AutoChangeTime()',1000);} 
void loop ( void ) {
  michome.running();
  lm.running();
  tlm.running();
  MUDP.running();

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
