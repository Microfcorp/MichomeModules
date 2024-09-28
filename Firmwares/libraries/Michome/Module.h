#ifndef Module_h
#define Module_h

#define ButtonUDP(m) (m.GetModule(0)+"_Button-%count")

#include <Arduino.h>
#include <LinkedList.h>
#include <FS.h>
#include <LittleFS.h>

typedef struct ModuleParam
{
    String Name;
    String Value;
};

typedef struct ModuleButton
{
    byte ButtonID;
    byte ButtonPin;
	String UDPEvent;
};

class ModuleClass {
    public:
		void addParam(String Name, String Value){Params.add((ModuleParam){Name, Value});}
		void addButton(byte ButtonID, byte ButtonPin, String UDPEvent){Buttons.add((ModuleButton){ButtonID, ButtonPin, UDPEvent});}
		
		int paramsCount() { return Params.size();}
		int buttonsCount() { return Buttons.size();}		
		
		ModuleParam getParam(int id) { return Params.get(id);}
		ModuleButton getButton(int id) { return Buttons.get(id);}
		
		void saveModuleParam(String path, ModuleParam &value){
			File f = LittleFS.open(path, "w");
			if (!f) {
				//Serial.println("Error open "+path+" for write");  //  "открыть файл не удалось"
				return;
			}
			else{
				f.write((byte *)&value, sizeof(value));
				f.close();                    
				return;
			}
		}
		
		ModuleParam loadModuleParam(String path){
			ModuleParam cf;
			
			File f = LittleFS.open(path, "r");
			if (!f) {
				//Serial.println("Error open "+path+" for read");  //  "открыть файл не удалось"
				cf.Name = "";
				cf.Value = "";				
				return cf;
			}
			else{
				f.read((byte *)&cf, sizeof(cf));
				f.close();	
				return cf;
			} 
		}
	private:
		LinkedList<ModuleParam> Params = LinkedList<ModuleParam>();           
		LinkedList<ModuleButton> Buttons = LinkedList<ModuleButton>();           
};

extern ModuleClass Module;

static uint16_t CountSymbols(String& str, char symbol){
	uint16_t counter = 0;
	for(int i = 0; i < str.length(); i++){
		if(str[i] == symbol) counter++;
	}
	return counter;
}

static String Split(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : data;
}

#endif