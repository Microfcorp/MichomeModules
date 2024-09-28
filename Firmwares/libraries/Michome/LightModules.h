#ifndef LightModules_h
#define LightModules_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

#include <Michom.h>
#include <LinkedList.h>
//#include <ArduinoJson.h>

#define LightModuleTimeoutConnection 500
#define EnableCRT
#define EffectsON

#ifdef EffectsON
	#define MaxEFParams 5
	//#define CreateEffect(name, desc, interval, mt, param1, param2, param3, param4, param5) (Effects.add((Effect){true, name, desc, interval, mt, false, param1, param2, param3, param4, param5}))
	#define CreateEffect(name, desc, interval, mt) (Effects.add((Effect){true, name, desc, interval, mt, false}))
	#define ForEveryPin() for(byte pinID = 0; pinID < (*Pins).size(); pinID++)
	#define GetPinMaxBrigth(pinID) (*Pins).get(pinID).MaxBrightnes();
	#define GetPinMinBrigth(pinID) (*Pins).get(pinID).MinBrightnes();
	//#define SetEveryPin(val, PinType) for(byte pinID = 0; pinID < (*Pins).size(); pinID++) {if((*Pins).get(pinID).Pin == PinType) { analogWrite((*Pins).get(pinID).Pin, val); LightPin lp = (*Pins).get(pinID); lp.value = val; (*Pins).set(pinID, lp); }}
	#define SetEveryPin(val) for(byte pinID = 0; pinID < (*Pins).size(); pinID++) { if((*Pins).get(pinID).Type == PWM) { analogWrite((*Pins).get(pinID).Pin, (*Pins).get(pinID).ConvertValue(val)); LightPin lp = (*Pins).get(pinID); lp.value = val; (*Pins).set(pinID, lp); } }
#endif

#if defined(EnableExternalUnits)
	#define NotWorkPin 0, 1, 2, 9, 16, 5, 4
	#define NotWorkPinCount 7
#else
	#define NotWorkPin 0, 1, 2, 9, 16
	#define NotWorkPinCount 5
#endif

#define MaximumBrightnes 1023
#define MinimumBrightnes 0

typedef enum LightType { SetLight, SetLightAll, Strobo, StroboPro, StroboAll, StroboAllPro, JSON, Debug };
typedef enum PinType { Relay, PWM };

typedef struct LightData
{
    LightType Type; //тип по таблицк
    int BR; //яркость
    uint8_t Pin; //пин
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
    uint8_t Pin; //пин
    PinType Type; //тип пина
	bool Reverse; //Реверс пина
	int value; //значение яркости
	
	int MaxBrightnes() const&{
		return (Type == PWM ? MaximumBrightnes : 1);
	}
	int MinBrightnes() const&{
		return MinimumBrightnes;
	}
	int ConvertValue(int val) const&{
		if(Type == Relay)
			return (val > 0 ? (Reverse ? LOW : HIGH) : (Reverse ? HIGH : LOW));
		else if(Type == PWM){
			val = min(val, MaxBrightnes());
			val = max(val, MinBrightnes());
			return (Reverse ? map(val, MinBrightnes(), MaxBrightnes(), MaxBrightnes(), MinBrightnes()) : val);
		}
		else
			return val;
	}
	int ValidateValue(int val) const&{
		if(Type == Relay)
			return (val > 0 ? HIGH : LOW);
		else if(Type == PWM){
			val = min(val, MaxBrightnes());
			val = max(val, MinBrightnes());
			return val;
		}
		else
			return val;
	}
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

//typedef std::function<void(LinkedList<LightPin> *Pins, ModuleParam *EFParams)> EffectHandlerFunction;
typedef std::function<void(LinkedList<LightPin> *Pins)> EffectHandlerFunction;
typedef struct Effect
{
	bool Enable; //Включен ли
    String Name; //название
    String Desc; //Описание
	int Interval; //Интервал выполнения
	EffectHandlerFunction voids; //Метод
	bool CurState; //Выполняется ли сейчас
	//ModuleParam Params[MaxEFParams]; //Параметры эффектов модуля
};

typedef std::function<void(uint8_t pinID, int State, int PreviewState)> PinStateHandlerFunction;

#ifdef EnableCRT
// функция возвращает скорректированное по CRT значение
// для 10 бит ШИМ
static int getBrightCRT(int val) {
	if(val < 30) return 0;
	return ((long)val * val + 1023) >> 10;
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
                void SetLight(LightPin &pin, int brith);
                //Установить яркость по ID
                bool SetLightID(byte pin, int brith);
				//Установить яркость по ID и вызов событий изменения из вне
                bool ExternalSetLightID(uint8_t pinID, int brith){
					#ifdef EffectsON
						StopAllEffect();
					#endif
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
					#ifdef EffectsON
						StopAllEffect();
					#endif
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
				//Возвращает, есть ли доступные данные в буфере
				bool IsBufferAvaliable(){
					return Bufers.size() > 0;
				}
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
					void InitEffects(); //Инициализация эффектов
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
                FadeData CreateFadeData(FadeType type, int Interval, int Pin, int MaxV, int MinV, bool ResetCerrent = true){
                  return {type, Interval, 0, Pin, min(MaxV, GetPin(Pin).MaxBrightnes()), min(MinV, GetPin(Pin).MinBrightnes()), (ResetCerrent ? (type == Down ? min(MaxV, GetPin(Pin).MaxBrightnes()) : min(MinV, GetPin(Pin).MinBrightnes())) : GetBrightness(Pin)), true};  
                };
				FadeData CreateFadeData(FadeType type, int Interval, int Pin, bool ResetCerrent = true){
                  return {type, Interval, 0, Pin, GetPin(Pin).MaxBrightnes(), GetPin(Pin).MinBrightnes(), (ResetCerrent ? (type == Down ? GetPin(Pin).MaxBrightnes() : GetPin(Pin).MinBrightnes()) : GetBrightness(Pin)), true};  
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
			/*void CreateEffect(String name, String desc, int interval, EffectHandlerFunction mt, ModuleParam param1 = {"",""}, ModuleParam param2 = {"",""}, ModuleParam param3 = {"",""}, ModuleParam param4 = {"",""}, ModuleParam param5 = {"",""}) 
			{
				Effects.add((Effect){true, name, desc, interval, mt, false, param1, param2, param3, param4, param5});
			}*/
			/*#ifdef EffectsON
			String GenerateHTMLParamsEF(byte EFid){
				Effect ef = Effects.get(EFid);
				String tmp = "";
				//for(byte i = 0; i < MaxEFParams; i++){
				//	ModuleParam par = (*ef.EffectParams).get(i);
				//	tmp += "<td>"+par.Name+": <input type='text' value='"+par.Value+"' onchange='postAjax(\"/effects?id="+(String)EFid+"&type=setparam&name="+par.Name+"&value=\"+this.value, GET, \"\", function(d){document.location.reload();})'' /></td>";
				//}
				return tmp;
			}
			String GenerateSaveParamsEF(byte EFid){
				//Effect ef = Effects.get(EFid);
				//String tmp = (String)((*ef.EffectParams).size()) + ":";
				//for(byte i = 0; i < (*ef.EffectParams).size(); i++){
				//	ModuleParam par = (*ef.EffectParams).get(i);
				//	tmp += par.Name+(String)"="+par.Value;
				//}
				return "";
			}
			Effect SetEFparam(Effect ef, String name, String value){
				//for(byte i = 0; i < (*ef.EffectParams).size(); i++){
				//	ModuleParam par = (*ef.EffectParams).get(i);
				//	if(par.Name == name){						
				//		par.Value = value;
				//		(*ef.EffectParams).set(i, par);
				//		break;
				//	}
				//}
				return ef;
			}
			#endif*/
			//RTOS SaveStateUpdate = RTOS(3000);//На сохранение
            
};
#endif // #ifndef LightModules_h