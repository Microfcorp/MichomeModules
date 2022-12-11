#include "MichomUDP.h"

void MichomeUDP::init(ESP8266WebServer& server2){
    UDP.beginMulticast(WiFi.localIP(), broadcast, MulticastUdpPort);
    Discover = RTOS(1000*60*2);
    Discover.Start();
    SendMulticast(GetData_Discover());    
    //UDP.beginMulticast(WiFi.localIP(), broadcast, localUdpPort);
    //replyPacekt = "Hi there! Got the message :-)";
    
    Load();
	
	ESP8266WebServer& server1 = server2;
    server1.on("/udptrigger", [&](){
        if(server1.arg("type") == "show" || !server1.hasArg("type")){
			if (!server1.chunkedResponseModeStart(200, "text/html")) {
				server1.send(505, F("text/html"), F("HTTP1.1 required"));
				return;
			}
			server1.sendContent(F("<head><meta charset=\"UTF-8\"><title>Настройка UDP триггеров</title></head>"));
			server1.sendContent(F("<table>"));			
            for(int i = 0; i < Ut.size(); i++){
                UDPTriggers tr = Ut.get(i);
                server1.sendContent((String)"<tr><form action='/udptrigger'><input name='type' type='hidden' value='save' /><input name='id' type='hidden' value='"+i+"' /><td>Состояние: <input type='checkbox' " + (tr.Enable == 1 ? "checked": "") +" name='en' /></td><td>Реагировать на себя: <input type='checkbox' " + (tr.EventFromMe == 1 ? "checked": "") +" name='evforme' /></td><td>Тип триггера <input name='TypeTrigger' value='"+tr.Type+"' /></td><td>Действие <select name='ActionType'>"+GetHTMLOptions((int)tr.ActionType)+"</select></td><td>Данные действия <input name='Data' value='"+tr.Data+"' /></td><td><input type='submit' value='Сохранить' /></td><td><a href='/udptrigger?type=remove&id="+i+"'>Удалить</a></td></form></tr>");
            }
            server1.sendContent(F("</table><br /><p><a href='udptrigger?type=add'>Добавить</a></p><p><a href='/'>Главная</a></p>"));
			server1.chunkedResponseFinalize();
        }
        else if(server1.arg("type") == "save"){
            int ids = server1.arg("id").toInt();
            bool en = server1.arg("en") == "on"; 
            bool evforme = server1.arg("evforme") == "on"; 
            String Types = server1.arg("TypeTrigger"); 
            ActionsType AT = (ActionsType)server1.arg("ActionType").toInt();
            String Data = server1.arg("Data");
            
            UDPTriggers tm = {Types, AT, Data, en, evforme};
            Ut.set(ids, tm);
            Save();
            server1.send(200, "text/html", (String)"<head><meta charset=\"UTF-8\"><meta http-equiv='refresh' content='1;URL=/udptrigger?type=show' /></head>Триггер "+ids+" сохранен");
        }
        else if(server1.arg("type") == "add"){  
            Ut.add({"EAlarm", SendGateway, "Test UDP", false, false});   
            Save();            
            server1.send(200, "text/html", (String)"<head><meta charset=\"UTF-8\"><meta http-equiv='refresh' content='1;URL=/udptrigger?type=show' /></head>Новый триггер добавлен");
        }
        else if(server1.arg("type") == "remove"){
            int ids = server1.arg("id").toInt();
            Ut.remove(ids);
            Save();
            server1.send(200, "text/html", (String)"<head><meta charset=\"UTF-8\"><meta http-equiv='refresh' content='1;URL=/udptrigger?type=show' /></head>Триггер "+ids+" удален");
        }
		else{
			server1.send(200, "text/html", (String)"<head><meta charset=\"UTF-8\"></meta></head>Ошибка комманды");
		}
    });
	
	on("Search", [&](IPAddress from, String cmd)
	{  
		if(IsStr(Split(cmd, '-', 1), type)){
			SendUDP(from, GetData_SearchOK());
			SendMulticast(GetData_SearchOK());
		}
		else if(Split(cmd, '-', 1) == "all"){
			SendMulticast(GetData_Discover());
		}
		/*else if(Split(cmd, '-', 1) == "noconfigured" && !(*gtw).IsConfigured){
			SendMulticast(GetData_SearchOK());
		}*/
	});
}

void MichomeUDP::Save(void){	
    int countQ = Ut.size();
    String sb = ((String)countQ) + "|";
    for(int i = 0; i < countQ; i++){
        UDPTriggers em = Ut.get(i);
        sb += String(em.Type) + "~" + String(em.ActionType) + "~" + String(em.Data) + "~" + String(em.Enable ? "1" : "0")+ String(em.EventFromMe ? "1" : "0") + "!";
    }                   
    fstext.WriteFile(sb);
}

void MichomeUDP::Load(void){
    String rd = fstext.ReadFile();
    int countQ = Split(rd, '|', 0).toInt();
    String data = Split(rd, '|', 1);
    for(int i = 0; i < countQ; i++){
        String str = Split(data, '!', i);
        UDPTriggers qq = {(Split(str, '~', 0)), ((ActionsType)Split(str, '~', 1).toInt()), (Split(str, '~', 2)), (Split(str, '~', 3).toInt() == 1), (Split(str, '~', 4).toInt() == 1)};
        Ut.add(qq);
    }
}

void MichomeUDP::SendUDP(IPAddress ip, String data){
    UDP.beginPacket(ip, UDP.remotePort());
      char msg[data.length()+1];
      (data).toCharArray(msg, data.length()+1);
    UDP.write(msg);
    UDP.endPacket();
}

void MichomeUDP::SendUDP(IPAddress ip, int Port, String data){
    UDP.beginPacket(ip, Port);
      char msg[data.length()+1];
      (data).toCharArray(msg, data.length()+1);
    UDP.write(msg);
    UDP.endPacket();
}

void MichomeUDP::SendMulticast(String data){
    UDP.beginPacketMulticast(broadcast, MulticastUdpPort, WiFi.localIP());
       char msg[data.length()+1];
      (data).toCharArray(msg, data.length()+1);
    UDP.write(msg);
    UDP.endPacket();
}

String MichomeUDP::Split(String data, char separator, int index)
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

static byte CountSymbols(String& str, char symbol){
	byte counter = 0;
	for(int i = 0; i < str.length(); i++){
		if(str[i] == symbol) counter++;
	}
	return counter;
}

void MichomeUDP::running(){
    int packetLength = UDP.parsePacket();
    if(packetLength > 1){                          
		int len = UDP.read(incomingPacket, 255);
		if (len > 0){
			incomingPacket[len] = 0;           
		}
		String reads = String(incomingPacket);
		reads.trim();
		
		if(UDP.remoteIP() != WiFi.localIP()){	
			String ns = Split(reads, '-', 0);
			for(int i = 0; i < Udata.size(); i++){
				if(ns == Udata.get(i).name){
					Udata.get(i).func(UDP.remoteIP(), reads);						
				}
			}
		}	
		if(Split(reads, '-', 0) == "Events"){
			if(Split(reads, '-', 1) == "list"){
				SendMulticast(GetData_EventsList());
			}
			else {
				for(int i = 0; i < Ut.size(); i++){
					UDPTriggers tr = Ut.get(i);
					if(tr.Enable && Split(reads, '-', 1) == tr.Type && ((!tr.EventFromMe && UDP.remoteIP() != WiFi.localIP()) || tr.EventFromMe)){
						String EVData = tr.Data;
						byte countParams = CountSymbols(reads, '-');
						for(byte i = 2, u = 1; i < countParams; i++, u++)
							EVData.replace((String)"%"+u, Split(reads, '-', i));
						if(tr.ActionType == (ActionsType)LightData && IsStr(type, StudioLight)){ForSDM(UDP.remoteIP(), EVData);}
						else if(tr.ActionType == (ActionsType)SendURL){ForURL(UDP.remoteIP(), EVData);}
						else if(tr.ActionType == (ActionsType)SendGateway){ForGTW(UDP.remoteIP(), EVData);} //"UDPData"
						else if(tr.ActionType == (ActionsType)SendsUDP){SendMulticast(EVData);}
						else if(tr.ActionType == (ActionsType)TimerData){ForTIM(UDP.remoteIP(), EVData);}
						else if(tr.ActionType == (ActionsType)LightScripts){ForLSC(UDP.remoteIP(), EVData);}
					}
				}
			}
		}
		//SendMulticast(reads);
        
    }
       
    if(Discover.IsTick()){  
        SendMulticast(GetData_Discover());
    }
}