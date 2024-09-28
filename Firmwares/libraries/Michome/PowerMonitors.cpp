#include "PowerMonitors.h"

//
// конструктор - вызывается всегда при создании экземпляра класса LightModules
//
PowerMonitorModules::PowerMonitorModules(Michome *m)
{
    gtw = m;
	(*gtw).SetOptionFirmware(PowerMonito, true);
	telnLM = &(*gtw).GetTelnet();
}

void PowerMonitorModules::init(){
	(*gtw).preInit();	
    
	if((*gtw).IsSaveMode) return;
	
	for(uint8_t i = 0; i < countPW; i++){
		LastPower[i] = {0, 0, 0};
		InitPowerMonitor(i);	
	}
	
    ESP8266WebServer& server1 = (*gtw).GetServer();
	  
      server1.on("/getpm", [&]() {
		uint8_t pmid = server1.hasArg("id") ? server1.arg("id").toInt() : 0;  
		PowerMonitorData tmp = _getPM(pmid);
        server1.send(200, "text/html", String(tmp.Voltage) + "," + String(tmp.Current) + "," + String(tmp.Power));
      });

      server1.on("/resetpm", [&]() {
		uint8_t termid = server1.hasArg("id") ? server1.arg("id").toInt() : 0;
		bool tmp = ResetPowerMonitor(termid);
        server1.send(200, "text/html", (tmp ? "Reset OK" : "Reset Error"));
      });

      server1.on("/initpm", [&]() {
		uint8_t termid = server1.hasArg("id") ? server1.arg("id").toInt() : 0;
        bool tmp = InitPowerMonitor(termid);
        server1.send(200, "text/html", (tmp ? "Init OK" : "Init Error"));
      });
	  
	  server1.on("/countpm", [&]() {
        server1.send(200, "text/html", (String)countPW);
      });
	  
	  server1.on("/pmenable", [&]() {
		  String tmp = "";
		  for(uint8_t i = 0; i < countPW; i++){
			if(EnablePowers[i])
				tmp += String(i) + ",";
		  }
		  tmp.remove(tmp.length()-1);
          server1.send(200, "text/html", tmp);
      });
	  
	  (*telnLM).on("gettemp","Reading temperatures {gettemp;0}");
	  (*telnLM).on("gettempinfo","Gets termometers informations {gettempinfo;0}");
	  (*telnLM).on("resettemp","Reset termometers {resettemp;0}");
	  (*telnLM).on("inittemp","Initializations termometers {inittemp;0}");	
	  (*telnLM).on("counttemp","Get count termometrs {gettemp;}");	
}

void PowerMonitorModules::TelnetRun(String telnd){
    /*String type = (*gtw).Split(telnd, ';', 0);
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
	}*/
}

void PowerMonitorModules::running(){
    if(TelnetEnable){              
        if((*telnLM).IsDataAvalible()){
            TelnetRun((*telnLM).GetData());
        }
    }
    
    if (rtos.IsTick()) {
		IsSendMinVoltage = false;
		IsSendFastVoltage = false;
		SendGateway();
	}
	if (fast_rtos.IsTick()) {
		for(uint8_t i = 0; i < countPW; i++){
			if(EnablePowers[i]){
				PowerMonitorData tmp = _getPM(i);
				PowerMonitorData lst = LastPower[i];
				if(lst.Voltage == 0 && lst.Current == 0 && lst.Power == 0){
					LastPower[i] = tmp;
				}
				else{
					if(abs(lst.Voltage - tmp.Voltage) > AlarmFastVoltage && !IsSendFastVoltage){
						IsSendFastVoltage = true;
						String temp = "[" + String(tmp.Voltage) + "," + String(tmp.Current) + "," + String(tmp.Power) + ",\"VoltageFastAlarm\"],";
						temp.remove(temp.length()-1);
						(*gtw).SendGateway((*gtw).GetModule(1), temp);
					}
					LastPower[i] = tmp;
				}
				if(tmp.Voltage <= AlarmMinimumVoltage && !IsSendMinVoltage){
					IsSendMinVoltage = true;
					String temp = "[" + String(tmp.Voltage) + "," + String(tmp.Current) + "," + String(tmp.Power) + ",\"VoltageMinAlarm\"],";
					temp.remove(temp.length()-1);
					(*gtw).SendGateway((*gtw).GetModule(1), temp);
				}
			}		
		}
	}
}

void PowerMonitorModules::SendGateway(){
	String temp = "";
	for(uint8_t i = 0; i < countPW; i++){
		if(EnablePowers[i]){
			PowerMonitorData tmp = _getPM(i);
			temp = temp + "[" + String(tmp.Voltage) + "," + String(tmp.Current) + "," + String(tmp.Power) + "],";
		}		
	}
	temp.remove(temp.length()-1);
	(*gtw).SendGateway((*gtw).GetModule(1), temp);
	((*gtw).GetUDP()).SendMulticast((*gtw).GetModule(0) + "-" + temp);
}

void PowerMonitorModules::ChangeTime(int time){
    rtos.ChangeTime(time);
}