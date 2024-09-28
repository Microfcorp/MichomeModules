#include <Michom.h>
#include <GyverButton.h>
#include <LightModules.h>
#include <TimerLightModule.h>

char* id = "StudioLight_Main";
const char* type = "StudioLight";
double VersionLightModule = 1.1;
/////////настройки//////////////

Michome michome(id, type, VersionLightModule);
LightModules lm (&michome);
TimerLightModule tlm(&lm);

RTOS rtos(605000);
RTOS rtos1(100);

ESP8266WebServer& server1 = michome.GetServer();

GButton butt1(5);

byte clicks = 0;

CreateMichome;

void setup ( void ) {
  michome.preInit();
  //lm.AddPin({12, PWM});
  //lm.AddPin({13, PWM});
  //lm.AddPin({15, PWM});

  if(!michome.IsSaveMode){
    FSFiles paramsPins("lightpins.txt");
    if(paramsPins.Exist()){ //Если есть данные пинов
      String vals = paramsPins.ReadFile();
      for(uint8_t i = 0; i < CountSymbols(vals, '|')+1; i++){
        String pinData = Split(vals, '|', i);
        if(pinData != ""){
          int pin = Split(pinData, '=', 0).toInt();
          PinType pinT = (PinType)Split(pinData, '=', 1).toInt();
          lm.AddPin({pin, pinT});
        }
      }
    }

    (michome.GetTelnet()).on("setlpins","changing lightPins data. Pin=Type|. Type: 0-Relay, 1-PWM (setlpins;12=1|13=1|15=1;)", []()
    {
      String data = (michome.GetTelnet()).GetData();
      String value = Split(data, ';', 1);
      FSFiles pinD("lightpins.txt");
      pinD.WriteFile(value);
     (michome.GetTelnet()).printSucess("Success changing lightPins to " + value);
    });
  }
  
  //pinMode (A0, INPUT);
  //pinMode (5, 2);
  butt1.setType(HIGH_PULL);
  //analogWriteFreq(3000);

  lm.TelnetEnable = true;
  //lm.SaveState = false;
  lm.init();
  tlm.init(); //Инициализация подсистемы времени

  michome.init(true);
  if(michome.IsSaveMode) return;  
  michome.TimeoutConnection = LightModuleTimeoutConnection;
 
}

void loop ( void ) {
  michome.running();
  if(michome.IsSaveMode) return;
  
  lm.running();
  tlm.running(); //Цикличная функция работы
  
  if (michome.GetSettingRead()) {
    rtos.ChangeTime(michome.GetSettingToInt("update"));
    rtos1.ChangeTime(michome.GetSettingToInt("adcread"));
    butt1.setClickTimeout(michome.GetSettingToInt("clicktimeout"));
    
    if (michome.GetSetting("logging") == "1")
      rtos.Start();
    else
      rtos.Stop();
  }

  if (rtos.IsTick()) {
    michome.SendGateway();
  }

  /*if (rtos1.IsTick()) {
    int KeyPress = keyboard.PresedKey();
    if (KeyPress == 0) {
      Run();
    }
    else if (KeyPress == 1) {
      
    }
    else if (KeyPress == 2) {
      Input();
    }
    else if (KeyPress == 3) {
      Mode();
    }
    else if (KeyPress == 4) {
      VolumeUp();
    }
    else if (KeyPress == 5) {
      VolumeDown();
    }
    
  } */
  
  butt1.tick();

  if (butt1.hasClicks()){clicks = butt1.getClicks(); michome.PortPrintln("Clicks="+String(clicks));}
  else clicks = 0;
 
  if (clicks == 3) {
    FadeData l1 = lm.CreateFadeData(Up, 15, 0, 168, 0);
    lm.StartFade(l1);
  }
  else if (clicks == 2) {
    lm.StopAllFade();
    lm.SetLightAll(0);
  }
  else if (clicks == 1) {
    FadeData l1 = lm.CreateFadeData(Up, 5, 0, false);
    FadeData l2 = lm.CreateFadeData(Up, 5, 1, false);
    FadeData l3 = lm.CreateFadeData(Up, 5, 2, false);
    lm.StartFade(l1);
    lm.StartFade(l2);
    lm.StartFade(l3);
  }
  if(clicks != 0 && clicks != 1 && clicks != 2 && clicks != 3) michome.SendGateway("get_button_press", "5="+String(clicks));    

  yield();
}
