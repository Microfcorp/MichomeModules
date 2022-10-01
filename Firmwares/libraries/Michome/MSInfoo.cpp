#include "MSInfoo.h"

//
// конструктор - вызывается всегда при создании экземпляра класса LightModules
//
MSInfoo::MSInfoo(Michome *m)
{
    gtw = m;
	(*gtw).SetOptionFirmware(MSInfoos, true);
	telnLM = &(*gtw).GetTelnet();
}

void MSInfoo::init(){
	(*gtw).preInit();	
    
	if((*gtw).IsSaveMode) return;
	
	InitMSInfoo();
	
    ESP8266WebServer& server1 = (*gtw).GetServer();
	  
      server1.on("/meteo", [&]() {
		  MSInfooData data = GetMSInfoo();
		  String tmp = "Температура 1: " + String(data.temp1) + "<br />";
		  tmp += "Температура 2: " + String(data.temp2) + "<br />";
		  tmp += "Влажность: " + String(data.humm) + "<br />";
		  tmp += "Давление: " + String(data.alt) + "<br />";
		  tmp += "Высота: " + String(data.press) + "<br />";
        server1.send(200, "text/html", RussianHead("Погодные данные") + tmp);
      });

      server1.on("/getmsinfoinfo", [&]() {
        server1.send(200, "text/html", RussianHead("Информация о метестанции") + String(GetMSInfooInfo()));
      });

      server1.on("/resettemp", [&]() {
		bool tmp = ResetMSInfoo();
        server1.send(200, "text/html", (tmp ? "Reset OK" : "Reset Error"));
      });

      server1.on("/inittemp", [&]() {
        bool tmp = InitMSInfoo();
        server1.send(200, "text/html", (tmp ? "Init OK" : "Init Error"));
      });
	  
	  (*telnLM).on("getmsinfoo","Reading temperatures {gettemp;}");
	  (*telnLM).on("getmsinfooinfo","Gets termometers informations {gettempinfo;}");
	  (*telnLM).on("resetmsinfoo","Reset termometers {resettemp;}");
	  (*telnLM).on("initmsinfoo","Initializations termometers {inittemp;}");
}

void MSInfoo::TelnetRun(String telnd){
    String type = (*gtw).Split(telnd, ';', 0);
	type.toLowerCase();
	if (type == "getmsinfoo") { //gettemp;
	  MSInfooData data = GetMSInfoo();
	  String tmp = "Temp 1: " + String(data.temp1);
	  tmp += "Temp 2: " + String(data.temp2);
	  tmp += "Humm: " + String(data.humm);
	  tmp += "Press: " + String(data.alt);
	  tmp += "Alt: " + String(data.press);
	  (*telnLM).printSucess(tmp);
	}
	else if (type == "getmsinfooinfo") { //gettempinfo;
	  (*telnLM).printSucess(String(GetMSInfooInfo()));
	}
	else if (type == "resetmsinfoo") { //resettemp;
	  bool tmp = ResetMSInfoo();
	  (*telnLM).printSucess((tmp ? "Reset OK" : "Reset Error"));
	}
	else if (type == "initmsinfoo") { //inittemp;
	  bool tmp = InitMSInfoo();
	  (*telnLM).printSucess((tmp ? "Init OK" : "Init Error"));
	}
}

void MSInfoo::running(){
    if(TelnetEnable){              
        if((*telnLM).IsDataAvalible()){
            TelnetRun((*telnLM).GetData());
        }
    }
    
    if (rtos.IsTick()) {
		SendData();
	}
}

void MSInfoo::SendData(){
	//  michome.SendData(michome.ParseJson(String(type), String(bmp.readPressure()/133.332)+";"+String(bmp.readTemperature())+";"+String(bmp.readAltitude())+";"+String(dht.readTemperature())+";"+String(dht.readHumidity())));
	MSInfooData data = GetMSInfoo();
	String tmp = "";
	tmp += String(data.press/133.332) + ";";
	tmp += String(data.temp2) + ";";
	tmp += String(data.alt) + ";";
	tmp += String(data.temp1) + ";";
	tmp += String(data.humm);	
    (*gtw).SendData((*gtw).ParseJson(tmp));
	
	tmp.replace(";", "-");
	
	((*gtw).GetUDP()).SendMulticast((*gtw).GetModule(0) + "-" + tmp);
}

void MSInfoo::ChangeTime(int time){
    rtos.ChangeTime(time);
}