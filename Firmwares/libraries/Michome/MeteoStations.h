#ifndef MeteoStations_h
#define MeteoStations_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

#define MeteoStationTimeoutConnection 4000

#ifndef MaxMeteo
	#define MaxMeteo 20
#endif

#ifndef TermometersValue
	#define TermometersValue float
#endif

#ifndef HummValue
	#define HummValue float
#endif

#ifndef PressValue
	#define PressValue float
#endif

#ifndef TermometersTypeValue
	#define TermometersTypeValue String
#endif

#ifndef HummTypeValue
	#define HummTypeValue String
#endif

#ifndef PressTypeValue
	#define PressTypeValue String
#endif

#include <Michom.h>

typedef enum TypeMeteo { Termo = 1, Hummidity = 2, Pressure = 4 };

constexpr bool IsSet(TypeMeteo val, TypeMeteo check) {
    return static_cast<TypeMeteo>(val) & static_cast<TypeMeteo>(check);
    // todo: пропущенные дополнительные проверки...
}

typedef struct MeteoStationData
{
	TermometersValue temp;
	HummValue humm;	
	PressValue press;
	TypeMeteo type;
};

typedef struct MeteoStationTypeValue
{
	TermometersTypeValue temp;
	HummTypeValue humm;	
	PressTypeValue press;	
	TypeMeteo type;
};

typedef std::function<bool(uint8_t idMeteo)> BTHandlerFunction;
typedef std::function<MeteoStationData(uint8_t idMeteo)> GTHandlerFunction;
typedef std::function<MeteoStationTypeValue(uint8_t idTermometrs)> GTTHandlerFunction;

class MeteoStationModules
{
        public:
                //Объявление класса
                MeteoStationModules(Michome *m);
                MeteoStationModules(){};
                
				void init();
				void running();
				void ChangeTime(int time);
				void SendGateway();
				
				void GetMeteoHandler(GTHandlerFunction th){_getTemp = th;}
				void GetMeteoInfoHandler(GTTHandlerFunction th){_getTermomersInfo = th;}
				void GetResetMeteoHandler(BTHandlerFunction th){_resetTemp = th;}
				void GetInitMeteoHandler(BTHandlerFunction th){_initTemp = th;}
				
				MeteoStationData GetMeteo(uint8_t idMeteo){return _getTemp(idMeteo);};
				MeteoStationTypeValue GetMeteoInfo(uint8_t idMeteo){return _getTermomersInfo(idMeteo);};
				bool ResetMeteo(uint8_t idMeteo){return _resetTemp(idMeteo);};
				bool InitMeteo(uint8_t idMeteo){ if(NeedResetForInit) ResetMeteo(idMeteo); EnableMeteo[idMeteo] = _initTemp(idMeteo); return EnableMeteo[idMeteo];};
				
				//Включение Telnet сервера
                bool TelnetEnable = false;
				//Необходим ли сброс при инициализации
                bool NeedResetForInit = false;
				//Установить количество термометров
				uint8_t SetCountMeteo(uint8_t count){count = min(count, (uint8_t)MaxMeteo); countMeteo = count; return countMeteo;};
				//Получить количество термометров
				uint8_t GetCountTermometrs(){return countMeteo;};
				//Использование термометров
				bool EnableMeteo[MaxMeteo];
        private:
            GTHandlerFunction _getTemp;
            GTTHandlerFunction _getTermomersInfo;
            BTHandlerFunction _resetTemp;
            BTHandlerFunction _initTemp;
			Michome *gtw;
			Telnet *telnLM;
			uint8_t countMeteo = 0;
			
			RTOS rtos = RTOS(600000);
			
			void TelnetRun(String telnd);
            
};
#endif // #ifndef MeteoStations_h