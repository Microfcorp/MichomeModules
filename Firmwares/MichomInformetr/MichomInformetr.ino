#include <Michom.h>
#include <InformetrModule.h>
#include "GyverButton.h"
#define IsLCDI2C

char* id = "Informetr_Pogoda";
const char* type = Informetr;
double VersionTermometr = 2.5;
/////////настройки//////////////

#define LightPin 12
#define ModePin 14
#define ButtonPin 16
#define LEDPin 10

RTOS Button14(500);//Опрос кнопки режимов
RTOS UpdateLight(50);//Включение подсветки
GButton button14(ModePin);//Кнопка смены режимов
GButton buttonUser(ButtonPin);//Пользовательская кнопка

Michome michome(id, type, VersionTermometr);
InformetrModule module(&michome);

ESP8266WebServer& server1 = michome.GetServer();

CreateMichome;

void setup(void) 
{
    pinMode(LightPin, INPUT_PULLUP); //Свет
    pinMode(ModePin, INPUT_PULLUP); //Кнопка
    pinMode(ButtonPin, INPUT_PULLUP); //Кнопка 2
    pinMode(LEDPin, OUTPUT); //лампоча
  
    module.SetDisplaySize(0x3F, 16, 2);      
    module.init();    
    michome.init(false);
    if(michome.IsSaveMode) return;
    
    UpdateLight.Start(); //Обновление света начать
    module.startUpdate();
}

void loop(void)
{
    michome.running();
    if(michome.IsSaveMode) return;
    
    module.running();    

    if (UpdateLight.IsTick()) //На опрос кнопки
      module.BacklightEnable = digitalRead(LightPin) == LOW;

    button14.tick(); //Кнопка режимов
    buttonUser.tick();

    if(button14.isSingle()){ //Если одна - меняем показ экрана
      module.InversePokazType();
    }
    else if(button14.isDouble()){ //Если 2 ставим на паузу
      //digitalWrite(LEDPin, 1);
      //String s = michome.SendToGateway("/michome/api/getprognoz.php?type=json");
      //Serial.println(s);
      //module.pause = !module.pause; 
      module.InverseHourlyDay();     
    }
    else if(button14.isTriple()){ //Если 3 - обновляем данные
      module.startUpdate();
    }
    else if(button14.isHolded()){ // Если удерживаем, то переходим режим часов
      module.InverseWorkMode();
    }

    if (buttonUser.hasClicks())
    {
      int clicks = buttonUser.getClicks(); 
      michome.PortPrint((String)clicks);
      michome.SendData(michome.ParseJson("get_button_press", String(ButtonPin)+"="+String(clicks)));
    }
    
    /*if(buttonUser.isSingle()){ //Если одна - меняем показ экрана
      
    }*/
}
