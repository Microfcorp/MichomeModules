#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <Michom.h>
#include <GyverButton.h>
#include <LightModules.h>
#include <TimerLightModule.h>
#include <MichomUDP.h>
#include <RC58.h>

//const char *ssid = "10-KORPUSMG";
//const char *password = "10707707";

const char* id = "StudioLight_Main";
const char* type = "StudioLight";
/////////настройки//////////////

const char* host = "192.168.1.42/michome/getpost.php";
const char* host1 = "192.168.1.42";

Michome michome(id, type);
LightModules lm (&michome);
TimerLightModule tlm(&lm);
MichomeUDP MUDP(&michome); //Создание класса UDP кнотроллера

RTOS rtos(605000);
RTOS rtos1(100);

ESP8266WebServer& server1 = michome.GetServer();
Telnet& telnet = michome.GetTelnet();

//Logger debug = michome.GetLogger();

const uint16_t kIrLed = 4;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.

int RStructures[] =
{ 0, 664,
  1, 734,
  2, 495,
  3, 778,
  4, 21,
  5, 813
};
RKeyboard keyboard(A0, RStructures, 6);

GButton butt1(5);

byte clicks = 0;

void handleIr() {
  for (uint8_t i = 0; i < server1.args(); i++) {
    if (server1.argName(i) == "code") {
      uint32_t code = strtoul(server1.arg(i).c_str(), NULL, 10);
      irsend.sendNEC(code, 32);
      delay(100);
      server1.send(200, "text/html", String(code));
    }
  }
}

void setup ( void ) {
  lm.AddPin({12, PWM});
  lm.AddPin({13, PWM});
  lm.AddPin({15, PWM});
  
  pinMode (A0, INPUT);
  //pinMode (5, 2);
  butt1.setType(HIGH_PULL);
  //analogWriteFreq(3000);

  lm.TelnetEnable = true;
  //lm.SaveState = false;
  lm.init();
  tlm.init(); //Инициализация подсистемы времени

  server1.on("/ir", handleIr);

  server1.on("/refresh", []() {
    server1.send(200, "text/html", "OK");
    michome.SendData();
  });

  michome.init(true);
  if(michome.IsSaveMode) return;  
  michome.TimeoutConnection = LightModuleTimeoutConnection;

  MUDP.lightModules = &lm; //Ссылка на объект модуля освещения
  MUDP.timerLightModules = &tlm; //Ссылка на объект подсисетмы времени
  MUDP.EAlarm = true; //Включено событие EAlarm
  MUDP.init(); //Инициализация модуля UDP
  
  irsend.begin();
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);

  Serial.println("Init ok");
}

void loop ( void ) {
  michome.running();
  if(michome.IsSaveMode) return;
  
  lm.running();
  tlm.running(); //Цикличная функция работы
  MUDP.running(); //Цикличная функция работы
  
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
    michome.SendData();
  }

  if (rtos1.IsTick()) {
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
    
  } 
  
  butt1.tick();

  if (butt1.hasClicks()){clicks = butt1.getClicks(); telnet.printlnNL("Clicks="+String(clicks));}
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
    FadeData l1 = lm.CreateFadeData(Up, 5, 0, 1024, 0);
    FadeData l2 = lm.CreateFadeData(Up, 5, 1, 1024, 0);
    FadeData l3 = lm.CreateFadeData(Up, 5, 2, 1024, 0);
    lm.StartFade(l1);
    lm.StartFade(l2);
    lm.StartFade(l3);
  }
  if(clicks != 0 && clicks != 1 && clicks != 2 && clicks != 3) michome.SendData(michome.ParseJson("get_button_press", "5="+String(clicks)));    

  yield();
}
