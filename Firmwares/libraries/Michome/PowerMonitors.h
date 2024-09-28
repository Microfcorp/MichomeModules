#ifndef PowerMonitors_h
#define PowerMonitors_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

#define PowerMonitorsTimeoutConnection 3000
#define MaxPowerMonitors 30

typedef struct PowerMonitorData
{
	float Voltage;
	float Current;	
	float Power;
};

#include <Michom.h>
//#include <LinkedList.h>
//#include <ArduinoJson.h>

typedef std::function<bool(uint8_t idPowerMonitor)> BPWHandlerFunction;
typedef std::function<PowerMonitorData(uint8_t idPowerMonitor)> GPWHandlerFunction;

class PowerMonitorModules
{
        public:
                //Объявление класса
                PowerMonitorModules(Michome *m);
                PowerMonitorModules(){};
                
				void init();
				void running();
				void ChangeTime(int time);
				void SendGateway();
				
				void GetPowerMonitorHandler(GPWHandlerFunction th){_getPM = th;}
				void GetResetMonitorHandler(BPWHandlerFunction th){_resetPM = th;}
				void GetInitMonitorHandler(BPWHandlerFunction th){_initPM = th;}
				
				PowerMonitorData GetPowerMonitor(uint8_t idPowerMonitor){ return _getPM(idPowerMonitor);};
				bool ResetPowerMonitor(uint8_t idPowerMonitor){return _resetPM(idPowerMonitor);};
				bool InitPowerMonitor(uint8_t idPowerMonitor){ if(NeedResetForInit) _resetPM(idPowerMonitor); EnablePowers[idPowerMonitor] = _initPM(idPowerMonitor); return EnablePowers[idPowerMonitor];};
								
				//Включение Telnet сервера
                bool TelnetEnable = false;
				//Необходим ли сброс при инициализации
                bool NeedResetForInit = false;
				//Граница 10 - секундного изменения напряжения
				float AlarmFastVoltage = 8.78;
				//Минимальная граница напряжения
				float AlarmMinimumVoltage = 3.8;
				//Установить количество термометров
				uint8_t SetCountPowerMonitors(uint8_t count){count = min(count, (uint8_t)MaxPowerMonitors); countPW = count; return countPW;};
				//Получить количество термометров
				uint8_t GetCountPowerMonitors(){return countPW;};
				bool EnablePowers[MaxPowerMonitors];
        private:
            GPWHandlerFunction _getPM;
            BPWHandlerFunction _resetPM;
            BPWHandlerFunction _initPM;
			Michome *gtw;
			Telnet *telnLM;
			uint8_t countPW = 0;
			bool IsSendMinVoltage = false;
			bool IsSendFastVoltage = false;
			PowerMonitorData LastPower[MaxPowerMonitors];
			
			RTOS rtos = RTOS(600000);
			RTOS fast_rtos = RTOS(RTOS10S);
			
			void TelnetRun(String telnd);
            
};
#endif // #ifndef PowerMonitors_h