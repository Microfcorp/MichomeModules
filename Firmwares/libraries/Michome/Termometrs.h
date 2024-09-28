#ifndef Termometrs_h
#define Termometrs_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

#define TermometrsTimeoutConnection 3000

#ifndef MaxTermometers
	#define MaxTermometers 8
#endif

#ifndef TermometersValue
	#define TermometersValue float
#endif

#ifndef TermometersTypeValue
	#define TermometersTypeValue String
#endif

#include <Michom.h>
//#include <LinkedList.h>
//#include <ArduinoJson.h>

typedef std::function<bool(uint8_t idTermometrs)> BTHandlerFunction;
typedef std::function<TermometersValue(uint8_t idTermometrs)> GTHandlerFunction;
typedef std::function<TermometersTypeValue(uint8_t idTermometrs)> GTTHandlerFunction;

class TermometrModules
{
        public:
                //Объявление класса
                TermometrModules(Michome *m);
                TermometrModules(){};
                
				void init();
				void running();
				void ChangeTime(int time);
				void SendGateway();
				
				void GetTempHandler(GTHandlerFunction th){_getTemp = th;}
				void GetTermomersInfoHandler(GTTHandlerFunction th){_getTermomersInfo = th;}
				void GetResetTempHandler(BTHandlerFunction th){_resetTemp = th;}
				void GetInitTempHandler(BTHandlerFunction th){_initTemp = th;}
				
				TermometersValue GetTemp(uint8_t idTermometrs){return _getTemp(idTermometrs);};
				TermometersTypeValue GetTermometersInfo(uint8_t idTermometrs){return _getTermomersInfo(idTermometrs);};
				bool ResetTemp(uint8_t idTermometrs){return _resetTemp(idTermometrs);};
				bool InitTemp(uint8_t idTermometrs){ if(NeedResetForInit) ResetTemp(idTermometrs); return _initTemp(idTermometrs);};
				
				//Включение Telnet сервера
                bool TelnetEnable = false;
				//Необходим ли сброс при инициализации
                bool NeedResetForInit = false;
				//Установить количество термометров
				uint8_t SetCountTermometrs(uint8_t count){count = min(count, (uint8_t)MaxTermometers); countTermometrs = count; return countTermometrs;};
				//Получить количество термометров
				uint8_t GetCountTermometrs(){return countTermometrs;};
        private:
            GTHandlerFunction _getTemp;
            GTTHandlerFunction _getTermomersInfo;
            BTHandlerFunction _resetTemp;
            BTHandlerFunction _initTemp;
			Michome *gtw;
			Telnet *telnLM;
			uint8_t countTermometrs = 0;
			
			RTOS rtos = RTOS(600000);
			
			void TelnetRun(String telnd);
            
};
#endif // #ifndef Termometrs_h