#include <Michom.h>
#include <WS28.h>

//#include <TimerLightModule.h>
//#include <MichomUDP.h>

const char* id = "WS28Module_Elka";
const char* type = WS28Module; //стандартный тип модуля освещения
/////////настройки//////////////

RTOS rtos(604000); //время опроса до сервера

RTOS rtosef(30); //

Michome michome(id, type); //инициальзация модуля Michome
WS28 ws (&michome, 50, 3); //Инициализация модуля освещения
//TimerLightModule tlm(&lm); //Инициализация подсистемы точного времени

ESP8266WebServer& server1 = michome.GetServer(); //Получение объекта веб-сервера

//MichomeUDP MUDP(&michome); //Создание класса UDP кнотроллера

//const int Keys[] = {9,10,4,5};

byte cefect = 0;
byte cepfect = 0;
byte rnds = 0;
bool isrunef = true;

void setup ( void ) {
  ws.TelnetEnable = true; //Включена поддержка telnet запросов
  //lm.SaveState = true; //Включено сохранение статуса выводов при перезапуске
  
  //tlm.init(); //Инициализация подсистемы времени
  michome.init(true); //Инициализация модуля Michome
  ws.init(); //Инициализация модуля освещения
  michome.TimeoutConnection = WS28ModuleTimeoutConnection; //Таймаут соединения до шлюза
  
  //MUDP.lightModules = &lm; //Ссылка на объект модуля освещения
  //MUDP.timerLightModules = &tlm; //Ссылка на объект подсисетмы времени
  //MUDP.EAlarm = true; //Включено событие EAlarm
  //MUDP.init(); //Инициализация модуля UDP
  
  server1.on("/refresh", [](){ 
    server1.send(200, "text/html", "OK");
    michome.SendData();
  });
  server1.on("/stop", [](){ 
    isrunef = false;
    server1.send(200, "text/html", "OK");
  });
  server1.on("/start", [](){
    isrunef = true; 
    server1.send(200, "text/html", "OK");
  });
}

void loop ( void ) {
  michome.running(); //Цикличная функция работы
  ws.running(); //Цикличная функция работы
  //tlm.running(); //Цикличная функция работы
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

  if (rtosef.IsTick() && isrunef) {
    if(cefect == 0){
      e1();
    }
    else if(cefect == 1){
      e2();
    }
    else if(cefect == 2){
      e4();
    }
    else{
      e3();
    }
  }
}
int i = 0;

void e1(){
  if(i == 0 && cepfect == 0){
    rtosef.ChangeTime(30);
    ws.SetLightAll(Black);
  }
  if(i < ws.CountPins()){
    rtosef.ChangeTime(30);
    i++;
    ws.SetLight(i, RgbColor(cepfect >= 0 ? 120 : 0, cepfect >= 2 ? 160 : 0, cepfect >= 1 ? 200 : 0));
  }
  else{
    i = 0;
    cepfect++;
    rtosef.ChangeTime(1500);
    if(cepfect >= 3){
      GenerateEfect();
    }
  }
}

void e2(){
  if(i == 0 && cepfect == 0){
    rtosef.ChangeTime(90);
    ws.SetLightAll(Black);
  }
  if(i < ws.CountPins()){
    i++;
    ws.SetLight(i, RgbColor(random(0, 160), random(0, 160), random(0, 160)));
  }
  else{
    i = 0;
    GenerateEfect();
  }
}

void e3(){
  if(i == 0 && cepfect == 0){
    rtosef.ChangeTime(80);
    ws.SetLightAll(Black);
  }
  if(i < ws.CountPins()){
    i++;
    int p = random(0, ws.CountPins());
    ws.SetLight(p, RgbColor(i < 15 ? 180 : 30, i < 35 ? 100 : 160, i > 35 ? 190 : 80));
  }
  else{
    i = 0;
    GenerateEfect();
  }
}

void e4(){
  if(i == 0){
    rtosef.ChangeTime(20);
    ws.SetLightAll(Black);
  }
  if(i < ws.CountPins()){
    i++;
    ws.SetLight(i, RgbColor(rnds >= 0 ? 200 : 0, rnds >= 2 ? 180 : 0, rnds >= 1 ? 200 : 0));
  }
  else{
    i = 0;
    GenerateEfect();
    /*cepfect++;
    if(cepfect >= 5){
      GenerateEfect();
    }*/
  }
}

void GenerateEfect(){
  cepfect = 0;
  i = 0;
  cefect = random(0, 5);
  Serial.println("Effect: " + String(cefect));
  rnds = random(0, 3);
}
