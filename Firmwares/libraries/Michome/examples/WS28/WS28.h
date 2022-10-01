#ifndef WS28_h
#define WS28_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

#define WS28ModuleTimeoutConnection 500

#include <Michom.h>
#include <LinkedList.h>
#include <NeoPixelBus.h>
//#include <ArduinoJson.h>

#define WS28Pin 3
#define MaximumBrightnes 255
#define MinimumBrightnes 0

#define Black (RgbColor (0))

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

typedef struct BufferData
{
    uint16_t Pin; //пин
    RgbColor value; //значение
};


class WS28
{
        public:
                //Объявление класса
                WS28(Michome *m, uint16_t LedCount, uint8_t Pin = WS28Pin);
                WS28(){};
                //
                //void RunData(LightData LD);
                //Установить яркость пина
                void SetLight(uint16_t ledid, RgbColor color);
                //Установить яркость всех
                void SetLightAll(RgbColor color);
                //Стробо по ID
                void Strobo(uint16_t ledid, RgbColor color, int col, int del);
                //СтробоПро по ID
                void StroboPro(uint16_t ledid, RgbColor color, int col, int del, int pdel);
                //Стробо все
                void StroboAll(RgbColor color, int col, int del);
                //СтробоПро все
                void StroboAllPro(RgbColor color, int col, int del, int pdel);                                
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
                bool AddBuferState(uint16_t ledid, RgbColor color);
                //Применить буфер
                void RunBuffer();
				//Получить текущую яркость пина
                //RgbColor GetBrightness(uint16_t ledid);
                //Возвращает количетво пинов
                int CountPins(){
                    return LedCount;
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
                //Сохранять состояние выходов при перезапуске
                bool SaveState = false;
				void DrawPixelsGamma(bool corrected, HslColor startColor, HslColor stopColor)
				{
					for (uint16_t index = 0; index < strip.PixelCount() - 1; index++)
					{
						float progress = index / static_cast<float>(strip.PixelCount() - 2);
						RgbColor color = HslColor::LinearBlend<NeoHueBlendShortestDistance>(startColor, stopColor, progress);
						if (corrected)
						{
							color = colorGamma.Correct(color);
						}
						strip.SetPixelColor(index, color);
					}

					// use the last pixel to indicate if we are showing corrected colors or not
					if (corrected)
					{
						strip.SetPixelColor(strip.PixelCount() - 1, RgbColor(64));
					}
					else
					{
						strip.SetPixelColor(strip.PixelCount() - 1, RgbColor(0));
					}

					strip.Show();
				}
        private:
            bool IsReadConfig = false;
            uint16_t LedCount = 50; 
			uint8_t Pin = WS28Pin;
            NeoPixelBus<NeoGrbFeature, NeoEsp8266Dma800KbpsMethod> strip = (50);
			NeoGamma<NeoGammaTableMethod> colorGamma;
            Michome *gtw;
			Telnet *telnLM;
            LinkedList<FadeData> Fades = LinkedList<FadeData>();
            LinkedList<BufferData> Bufers = LinkedList<BufferData>();
            int countfade = 0;
            
};
#endif // #ifndef WS28_h