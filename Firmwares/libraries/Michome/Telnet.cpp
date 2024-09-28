#include "Telnet.h"

//Объявление класса
Telnet::Telnet(int port, String ID){
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
void Telnet::Running(){
    //check if there are any new clients
    if (servT.hasClient()) {
        //find free/disconnected spot
        for(uint8_t i = 0; i < MAX_SRV_CLIENTS; i++){
            if (!serverClients[i] || !serverClients[i].connected()) {
                if (serverClients[i]) serverClients[i].stop();
                serverClients[i] = servT.available();
                //serverClients[i].setTimeout(100);
                serverClients[i].println((String)ansiBOLD + ansiCORS + ansiREDF + "Michome module Telnet" + ansiEND + "\n");
				InputArea(serverClients[i]);
            }
        }
    }    

    for(uint8_t i = 0; i < MAX_SRV_CLIENTS; i++){
        if (serverClients[i] && serverClients[i].connected()) {
            if (serverClients[i].available()) {
                RD = serverClients[i].readStringUntil('\n');    
				RD.trim();
				bool isfind = false;
				//InputArea(serverClients[i]);
				//serverClients[i].println(Split(RD, ';', 0));
				for(uint8_t t = 0; t < Ons.size(); t++){	
					if(CountSymbols(RD, ';') == 1 ? (RD.substring(0, RD.length()-1) == Ons.get(t).name) : (Split(RD, ';', 0) == Ons.get(t).name)){
						isfind = true;
						Ons.get(t).func();						
					}
				}
				if(!isfind) println();
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
	
	on(F("help"), F("Show help for commands"), [&](){
		printlnNIA(F("Commands for this Michome module: \n"));
        for(int i = 0; i < Ons.size(); i++){
			OnData data = Ons.get(i);
			printlnNIA(data.name + " \t - " + data.descreption);
		}
		println("");
    });
	
	on(F("restart"), F("Reboot module"), [&](){
		println(F("Module rebooting..."));
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