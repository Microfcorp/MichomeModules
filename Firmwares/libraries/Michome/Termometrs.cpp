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
	
	InitTemp();
	
    ESP8266WebServer& server1 = (*gtw).GetServer();
	  
      server1.on("/gettemp", [&]() {
        server1.send(200, "text/html", String(GetTemp()) + "C");
      });

      server1.on("/gettempinfo", [&]() {
        server1.send(200, "text/html", RussianHead("Информация о термодатчике") + String(GetTermometersInfo()));
      });

      server1.on("/resettemp", [&]() {
		bool tmp = ResetTemp();
        server1.send(200, "text/html", (tmp ? "Reset OK" : "Reset Error"));
      });

      server1.on("/inittemp", [&]() {
        bool tmp = InitTemp();
        server1.send(200, "text/html", (tmp ? "Init OK" : "Init Error"));
      });
	  
	  (*telnLM).on("gettemp","Reading temperatures {gettemp;}");
	  (*telnLM).on("gettempinfo","Gets termometers informations {gettempinfo;}");
	  (*telnLM).on("resettemp","Reset termometers {resettemp;}");
	  (*telnLM).on("inittemp","Initializations termometers {inittemp;}");	
}

void TermometrModules::TelnetRun(String telnd){
    String type = (*gtw).Split(telnd, ';', 0);
	type.toLowerCase();
	if (type == "gettemp") { //gettemp;
	  (*telnLM).printSucess(String(GetTemp()));
	}
	else if (type == "gettempinfo") { //gettempinfo;
	  (*telnLM).printSucess(String(GetTermometersInfo()));
	}
	else if (type == "resettemp") { //resettemp;
	  bool tmp = ResetTemp();
	  (*telnLM).printSucess((tmp ? "Reset OK" : "Reset Error"));
	}
	else if (type == "inittemp") { //inittemp;
	  bool tmp = InitTemp();
	  (*telnLM).printSucess((tmp ? "Init OK" : "Init Error"));
	}
}

void TermometrModules::running(){
    if(TelnetEnable){              
        if((*telnLM).IsDataAvalible()){
            TelnetRun((*telnLM).GetData());
        }
    }
    
    if (rtos.IsTick()) {
		SendData();
	}
}

void TermometrModules::SendData(){
	String temp = String(_getTemp());
    (*gtw).SendData((*gtw).ParseJson(temp));
	((*gtw).GetUDP()).SendMulticast((*gtw).GetModule(0) + "-" + temp);
}

void TermometrModules::ChangeTime(int time){
    rtos.ChangeTime(time);
}