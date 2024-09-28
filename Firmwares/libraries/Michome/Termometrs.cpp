#include "Termometrs.h"

//
// конструктор - вызывается всегда при создании экземпляра класса LightModules
//
TermometrModules::TermometrModules(Michome *m)
{
    gtw = m;
	(*gtw).SetOptionFirmware(Termometers, true);
	telnLM = &(*gtw).GetTelnet();
}

void TermometrModules::init(){
	(*gtw).preInit();	
    
	if((*gtw).IsSaveMode) return;
	
	for(uint8_t i = 0; i < countTermometrs; i++){
		InitTemp(i);
	}
	
    ESP8266WebServer& server1 = (*gtw).GetServer();
	  
      server1.on("/gettemp", [&]() {
		uint8_t termid = server1.hasArg("id") ? server1.arg("id").toInt() : 0;  
        server1.send(200, "text/html", String(GetTemp(termid)) + "C");
      });

      server1.on("/gettempinfo", [&]() {
		uint8_t termid = server1.hasArg("id") ? server1.arg("id").toInt() : 0;  
		bool isnonehtml = server1.hasArg("nonehtml") ? server1.arg("nonehtml") == "1" : false;  
        server1.send(200, "text/html", (!isnonehtml ? RussianHead("Информация о термодатчике") : (String)"") + String(GetTermometersInfo(termid)));
      });

      server1.on("/resettemp", [&]() {
		uint8_t termid = server1.hasArg("id") ? server1.arg("id").toInt() : 0;
		bool tmp = ResetTemp(termid);
        server1.send(200, "text/html", (tmp ? "Reset OK" : "Reset Error"));
      });

      server1.on("/inittemp", [&]() {
		uint8_t termid = server1.hasArg("id") ? server1.arg("id").toInt() : 0;
        bool tmp = InitTemp(termid);
        server1.send(200, "text/html", (tmp ? "Init OK" : "Init Error"));
      });
	  
	  server1.on("/counttemp", [&]() {
        server1.send(200, "text/html", (String)countTermometrs);
      });
	  
	  (*telnLM).on("gettemp","Reading temperatures {gettemp;0}");
	  (*telnLM).on("gettempinfo","Gets termometers informations {gettempinfo;0}");
	  (*telnLM).on("resettemp","Reset termometers {resettemp;0}");
	  (*telnLM).on("inittemp","Initializations termometers {inittemp;0}");	
	  (*telnLM).on("counttemp","Get count termometrs {gettemp;}");	
}

void TermometrModules::TelnetRun(String telnd){
    String type = (*gtw).Split(telnd, ';', 0);
	type.toLowerCase();
	if (type == "gettemp") { //gettemp;0
	  uint8_t termid = (*gtw).Split(telnd, ';', 1).toInt();
	  (*telnLM).printSucess(String(GetTemp(termid)));
	}
	else if (type == "gettempinfo") { //gettempinfo;0
	  uint8_t termid = (*gtw).Split(telnd, ';', 1).toInt();
	  (*telnLM).printSucess(String(GetTermometersInfo(termid)));
	}
	else if (type == "resettemp") { //resettemp;0
	  uint8_t termid = (*gtw).Split(telnd, ';', 1).toInt();
	  bool tmp = ResetTemp(termid);
	  (*telnLM).printSucess((tmp ? "Reset OK" : "Reset Error"));
	}
	else if (type == "inittemp") { //inittemp;0
	  uint8_t termid = (*gtw).Split(telnd, ';', 1).toInt();
	  bool tmp = InitTemp(termid);
	  (*telnLM).printSucess((tmp ? "Init OK" : "Init Error"));
	}
	else if (type == "counttemp") { //inittemp;
	  (*telnLM).printSucess((String)countTermometrs);
	}
}

void TermometrModules::running(){
    if(TelnetEnable){              
        if((*telnLM).IsDataAvalible()){
            TelnetRun((*telnLM).GetData());
        }
    }
    
    if (rtos.IsTick()) {
		SendGateway();
	}
}

void TermometrModules::SendGateway(){
	String temp = "";
	for(uint8_t i = 0; i < countTermometrs; i++){
		temp = temp + String(_getTemp(i)) + ",";		
	}
	temp.remove(temp.length()-1);
	(*gtw).SendGateway((*gtw).GetModule(1),temp);
	((*gtw).GetUDP()).SendMulticast((*gtw).GetModule(0) + "-" + temp);
}

void TermometrModules::ChangeTime(int time){
    rtos.ChangeTime(time);
}