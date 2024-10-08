#ifndef TimerLightModule_h
#define TimerLightModule_h

#define NoSaveHourAndMinutesForDayOFF //Включать таймер в 00:00 или во время выключения
#define SendCurrentTime //Показывать текущее время на странице настройки таймеров

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

#define MaxSyncPath 50

#if defined(FLASHMODE_DOUT)
	#define MaximumTimers 15
#else
	#define MaximumTimers 22
#endif

#define EFOffset 50

// IsScript = false, Pin: 0 - 255 Пин модуля
// IsScript = true, Pin: 0 - (EFOffset-1) Скрипт
// IsScript = true, Pin: EFOffset - 255 Эффект

typedef struct TimeLightModuleQ
{
    byte Hour;
    byte Minutes;
    byte Enable;
    bool IsScript;
    byte Pin;
    int State;
    bool IsDynamic;
	
	bool IsAutoOFF; //Автоматически отключать при изменении пина на сутки
	bool IsAutoON; //Автоматически включать, если измененное состояние ушло
	
	bool operator == (const TimeLightModuleQ & TLMQB) const {
	   return this->Hour == TLMQB.Hour && this->Minutes == TLMQB.Minutes && this->Enable == TLMQB.Enable && this->IsScript == TLMQB.IsScript && this->Pin == TLMQB.Pin && this->State == TLMQB.State && this->IsDynamic == TLMQB.IsDynamic && this->IsAutoOFF == TLMQB.IsAutoOFF;
	}
};

typedef struct DayStruct
{
    byte IdTLM;
    bool IsCurrent;
    byte ToOffDay;
	byte ToOffHour;
	byte ToOffMinute;
	
	int Pin;
	int State;
};

typedef struct TimerLightModuleSettings
{
    bool IsSync;
	char SyncPath[MaxSyncPath];
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
				bool ChangeForDay(byte id, bool state, int PinState = -1, bool isRun = true, bool isSave = true);
				//
				void RemoveChangeForDay(byte id);
				//
				int GetIDChangeForDay(byte TLMid);
				//
				void Change(byte id, bool state, bool isNeedSaveAndRun = true);
				//
				bool IsPlayTimer(TimeLightModuleQ em);
				//
				bool IsNeedEnableIsDay(byte TLMid, int pin, int br);
				//
				bool IsDayOff(byte TLMid);
        private:                       
            Michome *gtw;          
            LinkedList<TimeLightModuleQ> Qs = LinkedList<TimeLightModuleQ>();           
            LinkedList<DayStruct> Ds = LinkedList<DayStruct>();           
            LinkedList<uint8_t> ToDeleteDS = LinkedList<uint8_t>();
			LinkedList<TimeLightModuleQ> ToCurrentPin = LinkedList<TimeLightModuleQ>();			
            LightModules *light; 
			Telnet *telnet;
			MichomeUDP *udp;
			TimerLightModuleSettings M_settings;
            FSFiles fstext = FSFiles("/timer.txt");
            FSFiles fsdays = FSFiles("/timerdays.txt");
			RTOS timers = RTOS(RTOS1M);
			RTOS timers_sync = RTOS(RTOS1H);
            void _running();	
			void _synchroT();
            String GetPinsHTML(int pin, bool isScripts){
                String tmp = "";
                for(int i = 0; i < (*light).CountPins(); i++){
                    tmp += "<option "+(String)(!isScripts && i==pin ? "selected":"")+" value='"+(String)i+"'>"+(String)i+" ("+((*light).GetPin(i).Type == Relay ? "Реле" : "PWM")+")</option>";
                }
				for(int i = 0; i < (*light).GetCountScripts(); i++){
					LightScript sc = (*light).GetLightScript(i);
					if(sc.Enable)
						tmp += "<option "+(String)(isScripts && i==pin ? "selected":"")+" value='script_"+(String)i+"'>"+(String)sc.Name+" (Скрипт)</option>";
                }
				#ifdef EffectsON
				for(int i = 0; i < (*light).GetCountEffects(); i++){
					Effect ef = (*light).GetEffects(i);
					if(ef.Enable)
						tmp += "<option "+(String)(isScripts && i==(pin-EFOffset) ? "selected":"")+" value='ef_"+(String)i+"'>"+(String)ef.Name+" (Эффект)</option>";
                }
				#endif
                return tmp;
            }
			int GetIDTimer(TimeLightModuleQ em){
				for(int i = 0; i < Qs.size(); i++){
					if(Qs.get(i) == em)
						return i;
				}
				return -1;
			}
			int GetSyncPort(){
			    int httpPort = 80;
			    if(IsStr(String(M_settings.SyncPath), ":"))
					httpPort = Split(M_settings.SyncPath, ':', 1).toInt();
				return httpPort;
			}
			String GetSyncHost(){
			    return Split(M_settings.SyncPath, ':', 0);
			}
};
#endif // #ifndef TimerLightModule_h