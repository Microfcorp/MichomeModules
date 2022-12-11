#ifndef LightModules_h
#define LightModules_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

#define LightModuleTimeoutConnection 500
#define EnableCRT
#define EffectsON

#ifdef EffectsON
	#define CreateEffect(name, desc, interval, mt) (Effects.add({true, name, desc, interval, mt, false}))
	#define ForEveryPin() for(byte pinID = 0; pinID < (*Pins).size(); pinID++)
	#define SetEveryPin(val) for(byte pinID = 0; pinID < (*Pins).size(); pinID++) {analogWrite((*Pins).get(pinID).Pin, val); LightPin lp = (*Pins).get(pinID); lp.value = val; (*Pins).set(pinID, lp);}
#endif

#define NotWorkPin 0, 2, 9, 10
#define NotWorkPinCount 4

#include <Michom.h>
#include <LinkedList.h>
//#include <ArduinoJson.h>

#define MaximumBrightnes 1023
#define MinimumBrightnes 0

typedef enum LightType { SetLight, SetLightAll, Strobo, StroboPro, StroboAll, StroboAllPro, JSON, Debug };
typedef enum PinType { Relay, PWM };

typedef struct LightData
{
    LightType Type; //тип по таблицк
    int BR; //яркость
    byte Pin; //пин
    int Col; //количетво
    int Del; //задержка
    int PDel; //про-задержка
};

typedef enum FadeType { Up, Down, Stop };
typedef struct FadeData
{
    FadeType Type; //тип по таблицк
    int Interval; //время
    long CurMillis; //время
    int Pin; //пин
    int MaxV; //Максимальное значение
    int MinV; //Минимальное значение
    int CurV; //Текущее значение
    bool IsRun; //Выполняется ли
};

typedef struct LightPin
{	
    byte Pin; //пин
    PinType Type; //тип пина
	int value; //значение яркости
};

typedef struct BufferData
{
    byte Pin; //пин
    int Brig; //значение
};

typedef struct LightScript
{
	bool Enable; //Включен ли
    String Name; //название
    String Script; //Сам скрипт
};


typedef std::function<void(LinkedList<LightPin> *Pins)> EffectHandlerFunction;
typedef struct Effect
{
	bool Enable; //Включен ли
    String Name; //название
    String Desc; //Описание
	int Interval; //Интервал выполнения
	EffectHandlerFunction voids; //Метод
	bool CurState; //Выполняется ли сейчас
};

typedef std::function<void(uint8_t pinID, int State, int PreviewState)> PinStateHandlerFunction;

#ifdef EnableCRT
// функция возвращает скорректированное по CRT значение
// для 10 бит ШИМ
static int getBrightCRT(int val) {
  return ((long)val * val * val + 2094081) >> 20;
}
#endif

class LightModules
{
        public:
                //Объявление класса
                LightModules(Michome *m);
                LightModules(){};
                //Добавить ножку
                void AddPin(LightPin pin);
                //
                //void RunData(LightData LD);
                //Установить яркость пина
                void SetLight(byte pin, PinType type, int brith);
                //Установить яркость по ID
                bool SetLightID(byte pin, int brith);
				//Установить яркость по ID и вызов событий изменения из вне
                bool ExternalSetLightID(uint8_t pinID, int brith){
					StopAllEffect();
					int PrSt = GetBrightness(pinID);
					if(!SetLightID(pinID, brith)) return false;
					for(byte i=0; i < ExternalPinStateChanged.size(); i++)
						ExternalPinStateChanged.get(i)(pinID, brith, PrSt);
					return true;
				}
                //Установить яркость всех
                bool SetLightAll(int brith);
				//Установить яркость всех и вызов событий изменения из вне
                bool ExternalSetLightAll(int brith){
					StopAllEffect();
					int brs[Pins.size()];
					for(int a = 0; a < Pins.size(); a++)        
						brs[a] = GetBrightness(a);
						
					if(!SetLightAll(brith)) return false;
					for(byte i=0; i < ExternalPinStateChanged.size(); i++)
						for(int a = 0; a < Pins.size(); a++)        
							ExternalPinStateChanged.get(i)(a, brith, brs[a]);
					return true;
				}
				//Инвертирует состояние пина по ID
				void Reverse(byte id);
				void ExternalReverse(byte PinID){
					int PrSt = GetBrightness(PinID);
						
					Reverse(PinID);
					for(byte i=0; i < ExternalPinStateChanged.size(); i++)        
						ExternalPinStateChanged.get(i)(PinID, GetBrightness(PinID), PrSt);
				}
                //Стробо по ID
                void Strobo(byte pin, int col, int del);
                //СтробоПро по ID
                void StroboPro(byte pin, int col, int del, int pdel);
                //Стробо все
                void StroboAll(int col, int del);
                //СтробоПро все
                void StroboAllPro(int col, int del, int pdel);                                
                //Запустить плавное изменение яркости и получить его ID
                int StartFade(FadeData data);
                //Получить структуру плавного изменения яркости
                FadeData GetFade(int id);
                //Остановить плавное изменение яркости
                void StopFade(int id);
                //Остановить все плавные изменения яркости
                void StopAllFade();
                //Задать определенному ID структуру плавного изменения яркости
                void SetFade(int id, FadeData fd);
                //Добавить в буфер данные
                bool AddBuferState(byte pin, int brith);
                //Применить буфер
                void RunBuffer();
				//Получить текущую яркость пина
                int GetBrightness(byte pinid);
				//Выполнить скрипт освещения
				void RunScript(byte scriptID);
				//Возвращает число скриптов
				byte GetCountScripts(){return Scripts.size();};
				LightScript GetLightScript(byte id){return Scripts.get(id);};				
				#ifdef EffectsON
				//Выполнить эффект
				void RunEffect(byte effectID);
				//Остановить все эффекты
				void StopAllEffect();
				//Возвращает число эффектов
				byte GetCountEffects(){return Effects.size();};
				Effect GetEffects(byte id){return Effects.get(id);};
				#endif
                //Возвращает количетво пинов
                int CountPins(){
                    return Pins.size();
                }
                //Получить пин-дата
                LightPin GetPin(byte id){
                    return Pins.get(id);
                }
                //Создать структуру плавного изменения яркости
                FadeData CreateFadeData(FadeType type, int Interval, int Pin, int MaxV, int MinV){
                  return {type, Interval, 0, Pin, min(MaxV, MaximumBrightnes), min(MinV, MinimumBrightnes), min(MinV, MinimumBrightnes), true};  
                };
                //
                //String JSONParse(String text);
                //Инициализация класса
                void init();
                //Возвращает объект на шлюз
                Michome& GetMichome(){
                    return (*gtw);
                };
                //Выполнение всех операций
                void running();
                //Обработка Telnet данных
                void TelnetRun(String telndq);
                //Получить строку со всеми пинами
                String GetPins();
                //Включение Telnet сервера
                bool TelnetEnable = false;  
				//Включение UDP сервера
                bool UDPEnable = false; 
                //Сохранять состояние выходов при перезапуске
                bool SaveState = false;
				//Возникает при изменении состоянии выхода
				void OnPinChange(PinStateHandlerFunction action){PinStateChanged.add(action);};
				void OnExtPinChange(PinStateHandlerFunction action){ExternalPinStateChanged.add(action);};
				#ifdef EffectsON
				void InitEffects(); //Инициализация эффектов
				#endif
        private:
            bool IsReadConfig = false;
            //LinkedList<LightData> Datas = LinkedList<LightData>();
            LinkedList<LightPin> Pins = LinkedList<LightPin>();
            LinkedList<LightScript> Scripts = LinkedList<LightScript>();
			#ifdef EffectsON
            LinkedList<Effect> Effects = LinkedList<Effect>();
			RTOS EFTimer;
			#endif
            Michome *gtw;
			Telnet *telnLM;
			MichomeUDP *udpLM;
			FSFiles fstext = FSFiles("/lightscript.txt");
			FSFiles efdata = FSFiles("/efdata.txt");
            LinkedList<FadeData> Fades = LinkedList<FadeData>();
			LinkedList<uint8_t> ToDeleteFades = LinkedList<uint8_t>();
            LinkedList<BufferData> Bufers = LinkedList<BufferData>();
            int countfade = 0;
			byte CountSymbols(String& str, char symbol){
				byte counter = 0;
				for(int i = 0; i < str.length(); i++){
					if(str[i] == symbol) counter++;
				}
				return counter;
			}
			void Load();
			void Save();
			LinkedList<PinStateHandlerFunction> PinStateChanged = LinkedList<PinStateHandlerFunction>();
			LinkedList<PinStateHandlerFunction> ExternalPinStateChanged = LinkedList<PinStateHandlerFunction>();			
			//RTOS SaveStateUpdate = RTOS(3000);//На сохранение
            
};
#endif // #ifndef LightModules_h