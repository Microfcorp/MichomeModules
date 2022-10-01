#ifndef TimerLightModule_h
#define TimerLightModule_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 
#include <Michom.h>
#include <FSFiles.h>
#include <LightModules.h>
#include <MichomUDP.h>
#include <Telnet.h>
//#include <TimeSystem.h>

#define NTPTimer 60000
#define MaximumTimers 15

typedef struct TimeLightModuleQ
{
    byte Hour;
    byte Minutes;
    byte Enable;
    byte Pin;
    int State;
    bool IsDynamic;
};

typedef struct DayStruct
{
    byte IdTLM;
    bool IsCurrent;
    byte ToOffDay;
	byte ToOffHour;
	byte ToOffMinute;
};

class TimerLightModule
{
        public:
                //Объявление класса
                TimerLightModule(LightModules *m);
                TimerLightModule(){};
                //
                void running();
                //
                void Save();
                //
                void Load();
                //
                void init();
                //
                void Add(TimeLightModuleQ tm);
                //
				void ChangeForDay(byte id, bool state);
				//
				void Change(byte id, bool state);
        private:                       
            Michome *gtw;          
            LinkedList<TimeLightModuleQ> Qs = LinkedList<TimeLightModuleQ>();           
            LinkedList<DayStruct> Ds = LinkedList<DayStruct>();           
            LightModules *light; 
			Telnet *telnet;
			MichomeUDP *udp;
            FSFiles fstext = FSFiles("/timer.txt");
            FSFiles fsdays = FSFiles("/timerdays.txt");
			RTOS timers = RTOS(NTPTimer);
            void _running();
            String GetPinsHTML(int pin){
                String tmp = "";
                for(int i = 0; i < (*light).CountPins(); i++){
                    tmp += "<option "+(String)(i==pin ? "selected":"")+" value='"+(String)i+"'>"+(String)i+" ("+((*light).GetPin(i).Type == Relay ? "Relay" : "PWM")+")</option>";
                }
                return tmp;
            }
};
#endif // #ifndef TimerLightModule_h