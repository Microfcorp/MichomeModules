#include "Telnet.h"

//Объявление класса
Telnet::Telnet(int port, const char* ID){
    servT = WiFiServer(port);
	id = ID;
};
//
String Telnet::GetData(){
    if(!IsDataAvalible()) return "";

    String tm = RD;
    RD = NullString;
    return tm;
};
//
bool Telnet::IsDataAvalible(){
    return RD != NullString;
};
//
String Telnet::Split(String data, char separator, int index)
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

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
//
void Telnet::Running(){
    byte i;
    //check if there are any new clients
    if (servT.hasClient()) {
        //find free/disconnected spot
        for(i = 0; i < MAX_SRV_CLIENTS; i++){
            if (!serverClients[i] || !serverClients[i].connected()) {
                if (serverClients[i]) serverClients[i].stop();
                serverClients[i] = servT.available();
                //serverClients[i].setTimeout(100);
                serverClients[i].println((String)ansiBOLD + ansiCORS + ansiREDF + "Michome module Telnet" + ansiEND + "\n");
				InputArea(serverClients[i]);
            }
        }
    }    

    for(i = 0; i < MAX_SRV_CLIENTS; i++){
        if (serverClients[i] && serverClients[i].connected()) {
            if (serverClients[i].available()) {
                RD = serverClients[i].readStringUntil('\n');    
				RD.trim();
				bool isfind = false;
				//InputArea(serverClients[i]);
				for(int t = 0; i < Ons.size(); i++){
					if(Split(RD, ';', 0) == Ons.get(i).name){
						isfind = true;
						Ons.get(i).func();						
					}
				}
				if(!isfind) println("");
            }
        }   
    }    
};
typedef std::function<void(void)> THandlerFunction;
void Telnet::on(const String &linq, const String &descreption, THandlerFunction handler){
    Ons.add({linq, descreption, handler});    
};
//
void Telnet::Init(){
    servT.begin();
    servT.setNoDelay(true);
	
	on("help", "Show help for commands", [&](){
		printlnNIA("Commands for this Michome module");
        for(int i = 0; i < Ons.size(); i++){
			OnData data = Ons.get(i);
			printlnNIA(data.name + " \t - " + data.descreption);
		}
		println("");
    });
	
	on("restart", "Reboot module", [&](){
		println("Module rebooting...");
		ESP.restart();
    });
};
//
void Telnet::print(String text){
    for(byte i = 0; i < MAX_SRV_CLIENTS; i++){
        if (serverClients[i] && serverClients[i].connected()) {
			serverClients[i].print(text);
        }   
    }
};
//
void Telnet::println(String text){
    for(byte i = 0; i < MAX_SRV_CLIENTS; i++){
        if (serverClients[i] && serverClients[i].connected()) {
			serverClients[i].println(text);
			InputArea(serverClients[i]);
        }   
    }
};
void Telnet::printlnNL(String text){
    for(byte i = 0; i < MAX_SRV_CLIENTS; i++){
        if (serverClients[i] && serverClients[i].connected()) {
			serverClients[i].println("");
			serverClients[i].println(ansiEND + text);
			InputArea(serverClients[i]);
        }   
    }
};
//
void Telnet::printlnNIA(String text){
    for(byte i = 0; i < MAX_SRV_CLIENTS; i++){
        if (serverClients[i] && serverClients[i].connected()) {
			serverClients[i].println(text);
        }   
    }
};