#include "MeteoStations.h"

//
// конструктор - вызывается всегда при создании экземпляра класса MeteoStationModules
//
MeteoStationModules::MeteoStationModules(Michome *m)
{
    gtw = m;
	(*gtw).SetOptionFirmware(MeteoStations, true);
	telnLM = &(*gtw).GetTelnet();
}

void MeteoStationModules::init(){
	(*gtw).preInit();	
    
	if((*gtw).IsSaveMode) return;
	
	for(uint8_t i = 0; i < countMeteo; i++){
		EnableMeteo[i] = InitMeteo(i);
	}
	
    ESP8266WebServer& server1 = (*gtw).GetServer();
	  
      server1.on("/getmeteo", [&]() {
		uint8_t termid = server1.hasArg("id") ? server1.arg("id").toInt() : 0;  
		bool isnonehtml = server1.hasArg("nonehtml") ? server1.arg("nonehtml") == "1" : false;   
		MeteoStationData met = GetMeteo(termid);
        server1.send(200, "text/html", (!isnonehtml ? RussianHead("Метео информация") : (String)"") + (IsSet(met.type, TypeMeteo::Termo) ? String(met.temp) + "C" + (isnonehtml ? "\n" : "<br />") : "") + (IsSet(met.type, TypeMeteo::Hummidity) ? String(met.humm) + "%" + (isnonehtml ? "\n" : "<br />") : "") + (IsSet(met.type, TypeMeteo::Pressure) ? String(met.press) + " мм.рт.ст" : ""));
      });

      server1.on("/getmeteoinfo", [&]() {
		uint8_t termid = server1.hasArg("id") ? server1.arg("id").toInt() : 0;  
		bool isnonehtml = server1.hasArg("nonehtml") ? server1.arg("nonehtml") == "1" : false;  
		MeteoStationTypeValue met = GetMeteoInfo(termid);
        server1.send(200, "text/html", (!isnonehtml ? RussianHead("Информация о метеодатчике") : (String)"") + (IsSet(met.type, TypeMeteo::Termo) ? String(met.temp) + (isnonehtml ? "\n" : "<br />") : "") + (IsSet(met.type, TypeMeteo::Hummidity) ? String(met.humm) + (isnonehtml ? "\n" : "<br />") : "") + (IsSet(met.type, TypeMeteo::Pressure) ? String(met.press) : ""));
      });

      server1.on("/resetmeteo", [&]() {
		uint8_t termid = server1.hasArg("id") ? server1.arg("id").toInt() : 0;
		bool tmp = ResetMeteo(termid);
        server1.send(200, "text/html", (tmp ? "Reset OK" : "Reset Error"));
      });

      server1.on("/initmeteo", [&]() {
		uint8_t termid = server1.hasArg("id") ? server1.arg("id").toInt() : 0;
        bool tmp = InitMeteo(termid);
        server1.send(200, "text/html", (tmp ? "Init OK" : "Init Error"));
      });
	  
	  server1.on("/countmeteo", [&]() {
        server1.send(200, "text/html", (String)countMeteo);
      });
	  
	  server1.on("/meteoenable", [&]() {
		  String tmp = "";
		  for(uint8_t i = 0; i < countMeteo; i++){
			if(EnableMeteo[i])
				tmp += String(i) + ",";
		  }
		  tmp.remove(tmp.length()-1);
          server1.send(200, "text/html", tmp);
      });
	  
	  if(TelnetEnable){
		  (*telnLM).on("getmeteo","Reading meteo {getmeteo;0}");
		  (*telnLM).on("getmeteoinfo","Reading meteo informations {getmeteoinfo;0}");
		  (*telnLM).on("resetmeteo","Reset meteo {resetmeteo;0}");
		  (*telnLM).on("initmeteo","Initializations meteo {initmeteo;0}");	
		  (*telnLM).on("countmeteo","Get count meteo {countmeteo}");
		  (*telnLM).on("meteoenable","Get meteo enabled {meteoenable}");
	  }	  
}

void MeteoStationModules::TelnetRun(String telnd){
    String type = (*gtw).Split(telnd, ';', 0);
	type.toLowerCase();
	if (type == "getmeteo") { //gettemp;0
	  uint8_t termid = (*gtw).Split(telnd, ';', 1).toInt();
	  MeteoStationData met = GetMeteo(termid);
	  (*telnLM).printSucess((String)met.temp + "\n" + (String)met.humm + "\n" + (String)met.press);
	}
	else if (type == "getmeteoinfo") { //gettempinfo;0
	  uint8_t termid = (*gtw).Split(telnd, ';', 1).toInt();
	  MeteoStationTypeValue met = GetMeteoInfo(termid);
	  (*telnLM).printSucess(met.temp + "\n" + met.humm + "\n" + met.press);
	}
	else if (type == "resetmeteo") { //resettemp;0
	  uint8_t termid = (*gtw).Split(telnd, ';', 1).toInt();
	  bool tmp = ResetMeteo(termid);
	  (*telnLM).printSucess((tmp ? "Reset OK" : "Reset Error"));
	}
	else if (type == "initmeteo") { //inittemp;0
	  uint8_t termid = (*gtw).Split(telnd, ';', 1).toInt();
	  bool tmp = InitMeteo(termid);
	  (*telnLM).printSucess((tmp ? "Init OK" : "Init Error"));
	}
	else if (type == "countmeteo") { //inittemp;
	  (*telnLM).printSucess((String)countMeteo);
	}
	else if (type == "meteoenable") { //inittemp;
	  String tmp = "";
	  for(uint8_t i = 0; i < countMeteo; i++){
		if(EnableMeteo[i])
			tmp += String(i) + ",";
	  }
	  tmp.remove(tmp.length()-1);
	  (*telnLM).printSucess(tmp);
	}
}

void MeteoStationModules::running(){
    if(TelnetEnable){              
        if((*telnLM).IsDataAvalible()){
            TelnetRun((*telnLM).GetData());
        }
    }
    
    if (rtos.IsTick()) {
		SendGateway();
	}
}

void MeteoStationModules::SendGateway(){
	String temp = "";
	for(uint8_t i = 0; i < countMeteo; i++){
		if(EnableMeteo[i]){
			MeteoStationData met = _getTemp(i);
			temp = temp + "[" + String(met.temp) + "," + String(met.humm) + "," + String(met.press) + "],";		
		}
	}
	temp.remove(temp.length()-1);
	(*gtw).SendGateway((*gtw).GetModule(1), temp);
	((*gtw).GetUDP()).SendMulticast((*gtw).GetModule(0) + "-" + temp);
}

void MeteoStationModules::ChangeTime(int time){
    rtos.ChangeTime(time);
}