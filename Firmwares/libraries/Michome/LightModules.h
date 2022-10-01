#ifndef LightModules_h
#define LightModules_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

#define LightModuleTimeoutConnection 500

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
                void SetLightID(byte pin, int brith);
                //Установить яркость всех
                void SetLightAll(int brith);
				//Инвертирует состояние пина по ID
				void Reverse(byte id);
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
                  return {type, Interval, 0, Pin, MaxV, MinV, MinV, true};  
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
                void TelnetRun(String telnd);
                //Получить строку со всеми пинами
                String GetPins();
                //Включение Telnet сервера
                bool TelnetEnable = false;  
				//Включение UDP сервера
                bool UDPEnable = false; 
                //Сохранять состояние выходов при перезапуске
                bool SaveState = false;
        private:
            bool IsReadConfig = false;
            //LinkedList<LightData> Datas = LinkedList<LightData>();
            LinkedList<LightPin> Pins = LinkedList<LightPin>();
            Michome *gtw;
			Telnet *telnLM;
			MichomeUDP *udpLM;
            LinkedList<FadeData> Fades = LinkedList<FadeData>();
            LinkedList<BufferData> Bufers = LinkedList<BufferData>();
            int countfade = 0;
			//RTOS SaveStateUpdate = RTOS(3000);//На сохранение
            
};
#endif // #ifndef LightModules_h