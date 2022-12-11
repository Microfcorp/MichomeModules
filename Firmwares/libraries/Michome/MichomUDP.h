#ifndef MichomUDP_h
#define MichomUDP_h

#define IsStr(str, str2) ((str.indexOf(str2) != -1))

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <LinkedList.h>
#include <RTOS.h>
#include <FSFiles.h>
#include <ModuleTypes.h>

typedef enum ActionsType {LightData, SendURL, SendGateway, SendsUDP, TimerData, LightScripts};

typedef struct UDPTriggers
{
    String Type;
    ActionsType ActionType;
    String Data;
    bool Enable;
	bool EventFromMe; //реагирует ли на себя самого
};

typedef std::function<void(IPAddress from, String cmd)> UDPTHandlerFunction;

typedef struct UDPData
{
    String name;
    UDPTHandlerFunction func;
};

class MichomeUDP
{
        public:
            String id; String type;
            
            MichomeUDP(String _id, String _type){
                id = _id;
                type = _type;
            }
            
            void init(ESP8266WebServer& server2);

            void running(void);
            
            void Save(void);
            
            void Load(void);
            
            void SendUDP(IPAddress ip, String data);
            
            void SendUDP(IPAddress ip, int Port, String data);
            
			void on(String name, UDPTHandlerFunction func){
				Udata.add({name, func});
			}
			
            String Split(String data, char separator, int index);
            
            String GetData_Discover(){
                return "Michome_" + WiFi.localIP().toString();
            }
            
            String GetData_SearchOK(){
                return "SearchOK_" + type + "-" + id + "-" + WiFi.localIP().toString();
            }
            
            String GetData_MyType(){
                return "Module_" + id + "-" + type;
            }
            
            String GetData_OK(){
                return type + "-" + id + "_OK";
            }
            
            String GetData_EventsList(){
                String tmp = "";                           
                for(int i = 0; i < Ut.size(); i++){
                    UDPTriggers tr = Ut.get(i);
                    tmp += tr.Type + "|";
                }  
                return (tmp == "" ? "None Events" : tmp);
            }
            
            String GetData_Event_OK(String Event){
                return "Event_"+Event+"_OK";
            }
            
            String GetHTMLOptions(int t){
                String tmp = "";
                tmp += (IsStr(type, StudioLight) ? (String)"<option "+(t == 0 ? "selected":"")+" value='0'>Telnet формат модуля освещения</option>" : "");
                tmp += (String)"<option "+(t == 1 ? "selected":"")+" value='1'>Отпрвить URL</option>";
                tmp += (String)"<option "+(t == 2 ? "selected":"")+" value='2'>Отправить данные на шлюз</option>";
                tmp += (String)"<option "+(t == 3 ? "selected":"")+" value='3'>Отправить по UDP сети</option>";
                tmp += (String)"<option "+(t == 4 ? "selected":"")+" value='4'>Управление таймерами</option>";
                tmp += (IsStr(type, StudioLight) ? (String)"<option "+(t == 5 ? "selected":"")+" value='5'>Выполнить скрипт освещения</option>" : "");
                return tmp;
            }
			
			void SendTrigger(String Trigger, String Data = ""){
				SendMulticast("Events-" + Trigger + "-" + Data);
			}
            
            void SendMulticast(String data);
			
			void EventStudioLight(UDPTHandlerFunction action){ForSDM = action;};
			void EventSendGateway(UDPTHandlerFunction action){ForGTW = action;};
			void EventSendURL(UDPTHandlerFunction action){ForURL = action;};
			void EventTimersData(UDPTHandlerFunction action){ForTIM = action;};
			void EventLightScript(UDPTHandlerFunction action){ForLSC = action;};
        private:
            WiFiUDP UDP;
            unsigned int localUdpPort = 4210;
            unsigned int MulticastUdpPort = 4244;
            char incomingPacket[255];
            char replyPacekt[30];
            FSFiles fstext = FSFiles("/UDP.txt");
            IPAddress broadcast = IPAddress(224, 0, 1, 3); //224.0.1.3
            RTOS Discover;
            LinkedList<UDPTriggers> Ut = LinkedList<UDPTriggers>();
            LinkedList<UDPData> Udata = LinkedList<UDPData>();
			
			UDPTHandlerFunction ForSDM;
			UDPTHandlerFunction ForGTW;
			UDPTHandlerFunction ForURL;
			UDPTHandlerFunction ForTIM;
			UDPTHandlerFunction ForLSC;
};
#endif // #ifndef MichomUDP_h