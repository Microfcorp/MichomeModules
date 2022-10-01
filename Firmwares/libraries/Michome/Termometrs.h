#ifndef LightModules_h
#define LightModules_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

#define TermometrsTimeoutConnection 3000

#define TermometersValue float
#define TermometersTypeValue String

#include <Michom.h>
//#include <LinkedList.h>
//#include <ArduinoJson.h>

typedef std::function<bool(void)> BTHandlerFunction;
typedef std::function<TermometersValue(void)> GTHandlerFunction;
typedef std::function<TermometersTypeValue(void)> GTTHandlerFunction;

class TermometrModules
{
        public:
                //Объявление класса
                TermometrModules(Michome *m);
                TermometrModules(){};
                
				void init();
				void running();
				void ChangeTime(int time);
				void SendData();
				
				void GetTempHandler(GTHandlerFunction th){_getTemp = th;}
				void GetTermomersInfoHandler(GTTHandlerFunction th){_getTermomersInfo = th;}
				void GetResetTempHandler(BTHandlerFunction th){_resetTemp = th;}
				void GetInitTempHandler(BTHandlerFunction th){_initTemp = th;}
				
				TermometersValue GetTemp(){return _getTemp();};
				TermometersTypeValue GetTermometersInfo(){return _getTermomersInfo();};
				bool ResetTemp(){return _resetTemp();};
				bool InitTemp(){ if(NeedResetForInit) ResetTemp(); return _initTemp();};
				
				//Включение Telnet сервера
                bool TelnetEnable = false;
				//Необходим ли сброс при инициализации
                bool NeedResetForInit = false; 				
				
        private:
            GTHandlerFunction _getTemp;
            GTTHandlerFunction _getTermomersInfo;
            BTHandlerFunction _resetTemp;
            BTHandlerFunction _initTemp;
			Michome *gtw;
			Telnet *telnLM;
			
			RTOS rtos = RTOS(600000);
			
			void TelnetRun(String telnd);
            
};
#endif // #ifndef LightModules_h