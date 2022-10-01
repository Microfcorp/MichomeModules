#ifndef LightModules_h
#define LightModules_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

#define TermometrsTimeoutConnection 3000

#define TermometersValue float
#define HummValue int
#define AltValue float
#define PressureValue float
#define TermometersTypeValue String

#include <Michom.h>
//#include <LinkedList.h>
//#include <ArduinoJson.h>

typedef struct MSInfooData
{
	TermometersValue temp1;
	TermometersValue temp2;
	HummValue humm;
	AltValue alt;
	PressureValue press;	
};

typedef std::function<bool(void)> BTHandlerFunction;
typedef std::function<MSInfooData(void)> GTHandlerFunction;
typedef std::function<TermometersTypeValue(void)> GTTHandlerFunction;

class MSInfoo
{
        public:
                //Объявление класса
                MSInfoo(Michome *m);
                MSInfoo(){};
                
				void init();
				void running();
				void ChangeTime(int time);
				void SendData();
				
				void SetMSInfooHandler(GTHandlerFunction th){_getTemp = th;}
				void SetMSInfooInfoHandler(GTTHandlerFunction th){_getTermomersInfo = th;}
				void SetResetMSInfooHandler(BTHandlerFunction th){_resetTemp = th;}
				void SetInitMSInfooHandler(BTHandlerFunction th){_initTemp = th;}
				
				MSInfooData GetMSInfoo(){return _getTemp();};
				TermometersTypeValue GetMSInfooInfo(){return _getTermomersInfo();};
				bool ResetMSInfoo(){return _resetTemp();};
				bool InitMSInfoo(){ if(NeedResetForInit) ResetMSInfoo(); return _initTemp();};
				
				TermometersValue GetTemperature1(){ return GetMSInfoo().temp1;}
				TermometersValue GetTemperature2(){ return GetMSInfoo().temp2;}
				HummValue GetHumm(){ return GetMSInfoo().humm;}
				AltValue GetAlt(){ return GetMSInfoo().alt;}
				PressureValue GetPressure(){ return GetMSInfoo().press;}
				
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