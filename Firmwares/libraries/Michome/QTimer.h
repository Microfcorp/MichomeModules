#ifndef TimerLightModule_h
#define TimerLightModule_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 
#include <Michom.h>
#include <LightModules.h>
#include <NTPClient.h>
#include <EEPROM.h>

#define UTC3 10800; //Utc+3

typedef struct TimeQ
{
    
}

class TimerLightModule
{
        public:
                //Объявление класса
                TimerLightModule(Michome m, LightModules ls){
                    gtw = m;
                    light = ls;
                }
                //
                String GetData();
                //
                bool IsDataAvalible();
                //
                void Running(){
                    
                }
                //
                void print(String text);
                //
                void println(String text);
                //
                void Init(){
                    
                }
                
        private:
            WiFiUDP ntpUDP;
            NTPClient timeClient(ntpUDP, "pool.ntp.org", UTC3);
            Michome gtw;
            LightModules light;
            
};
#endif // #ifndef QTimer_h