#ifndef PinWatcher_h
#define PinWatcher_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

#include <Telnet.h>
#include <LinkedList.h>
#include <RTOS.h>
#include <Module.h>
//#include <ArduinoJson.h>

typedef struct PinWatcherData
{
	uint8_t pin;
	bool state;	
};

//typedef std::function<bool(void)> BTHandlerFunction;
typedef std::function<void(uint8_t pin, bool state)> PWHandlerFunction;

class PinWatcher
{
        public:
                //Объявление класса
                PinWatcher(Telnet *t){telnLM = t;};
                
				void init(){
					ModuleParam fromSave = Module.loadModuleParam("PinWatcher.bin");
					if(fromSave.Value != ""){
						uint8_t countParams = CountSymbols(fromSave.Value, ',');
						for(uint8_t i = 0; i < countParams+1; i++){
							uint8_t pin = Split(fromSave.Value, ',', i).toInt();
							pinMode(pin, INPUT_PULLUP);
							PinsW.add(pin);
						}
					}
					
					(*telnLM).on("addpinwatcher", "Added pin for watcher", [&](){
						String data = (*telnLM).read();
						uint8_t pin = Split(data, ';', 1).toInt();
						pinMode(pin, INPUT_PULLUP);
						PinsW.add(pin);
						(*telnLM).println("OK");
						save();
					});
					
					(*telnLM).on("removepinwatcher", "Remove pin for watcher", [&](){
						String data = (*telnLM).read();
						remove(Split(data, ';', 1).toInt());
						(*telnLM).println("OK");
						save();
					});
					
					(*telnLM).on("getpinwatcher", "Get pin for watcher", [&](){
						if(PinsW.size() == 0){
							(*telnLM).println("None pin on watcher");
						}
						else{
							for(uint8_t i = 0; i < PinsW.size(); i++){
								(*telnLM).println("Pin " + String(PinsW.get(i)) + " - is watcher");
							}
						}
						//(*telnLM).println("");
					});
					
					timerC.Start();
				}
				void running(){
					if(timerC.IsTick()){
						for(uint8_t i = 0; i < PinsW.size(); i++){
							if(digitalRead(PinsW.get(i)) == LOW){
								(*telnLM).println("Pin " + String(PinsW.get(i)) + " - is trigger");
								_mainH(PinsW.get(i), LOW);
							}
						}
					}
				}
				
				void save(){
					String tmp = "";
					for(uint8_t i = 0; i < PinsW.size(); i++){
						tmp += String(PinsW.get(i)) + ",";
					}
					tmp.remove(tmp.length()-1);
					String path = "PinWatcher.bin";
					ModuleParam dat = {String("PinWatcher"), tmp};
					Module.saveModuleParam(path, dat);
				};
				
				void remove(uint8_t Pin){
					for(uint8_t i = 0; i < PinsW.size(); i++){
						if(PinsW.get(i) == Pin){
							PinsW.remove(i);
						}
					}
				}
				
				void SetPinWatcherHandler(PWHandlerFunction th){_mainH = th;}												
        private:
            PWHandlerFunction _mainH;
			Telnet *telnLM;		
			LinkedList<uint8_t> PinsW = LinkedList<uint8_t>();
			RTOS timerC = RTOS(500);
};
#endif // #ifndef PinWatcher_h