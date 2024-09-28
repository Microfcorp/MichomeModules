#include "OLEDModule.h"

//
// конструктор - вызывается всегда при создании экземпляра класса OLEDModule
//
OLEDModule::OLEDModule(Michome *m)
{
    gtw = m;
	(*gtw).SetOptionFirmware(OLEDModules, true);
}

void OLEDModule::init()
{
		(*gtw).preInit();
		if((*gtw).IsSaveMode)
			return;
		
		Load();
		
		ESP8266WebServer& server1 = (*gtw).GetServer();
		server1.on("/onlight", [&]() {
			UpdateLight(true, Setting.DisplayBR);
			server1.send(200, F("text/html"), F("OK"));
		});

		server1.on("/offlight", [&]() {
			UpdateLight(false, Setting.DisplayBR);
			server1.send(200, F("text/html"), F("OK"));
		});

		(*gtw).SetRefreshData([&]() {
			server1.send(200, F("text/html"), F("OK"));
			startUpdate();
		});

		server1.on("/pause", [&]() {
			if(server1.arg(0) == "stop")
				pause = true;
			else if(server1.arg(0) == "run")
				pause = false;
			else pause = !pause;
			server1.send(200, F("text/html"), (String)pause + " - OK");
		});

		/*server1.on("/print", [&]() {
			lcd.clear();
			lcd.home();
			lcd.setCursor(0, 0);
			lcd.print(server1.arg("fl"));
			lcd.setCursor(0, 1);
			lcd.print(server1.arg("sl"));
			server1.send(200, "text/html", String("OK"));
		});*/

		/*server1.on("/test", [&]() {
			lcd.noBacklight();
			lcd.backlight();
			//ToPrognoz();

			lcd.clear();
			lcd.write(0);
			lcd.write(1);
			lcd.write(2);
			lcd.write(3);
			lcd.write(4);
			lcd.write(5);

			server1.send(200, "text/html", "System OK");
		});*/
		
		server1.on("/setdata", [&]() {
			parse(server1.arg(0));
			server1.send(200, "text/html", "OK");
		});
		server1.on("/oled", [&]() {
			if(server1.arg("type") == "show" || !server1.hasArg("type")){
				if (!server1.chunkedResponseModeStart(200, "text/html")) {
					server1.send(505, F("text/html"), F("HTTP1.1 required"));
					return;
				}
				server1.sendContent(F("<head>"));
				server1.sendContent(Styles);
				server1.sendContent(MainJS);
				server1.sendContent(AJAXJs);
				server1.sendContent(F("<script>function GetDataINF(){postAjax('/oled?type=get', GET, '', function(d){ab = d.split(';'); document.getElementById(\"startD\").value = (parseInt(ab[0]) < 10 ? '0' : '') + ab[0] + ':' + (parseInt(ab[1]) < 10 ? '0' : '') + ab[1]; document.getElementById(\"stopD\").value = (parseInt(ab[2]) < 10 ? '0' : '') + ab[2] + ':' + (parseInt(ab[3]) < 10 ? '0' : '') + ab[3]; document.getElementById(\"BR\").value = ab[4]});} function sendTimeD(){var sth = document.getElementById(\"startD\").value.split(':')[0]; var stm = document.getElementById(\"startD\").value.split(':')[1]; var soh = document.getElementById(\"stopD\").value.split(':')[0]; var som = document.getElementById(\"stopD\").value.split(':')[1]; var br = document.getElementById(\"BR\").value; postAjax('/oled?type=set&sth='+sth+'&stm='+stm+'&soh='+soh+'&som='+som+'&br='+br, GET, '', function(d){}); } window.setTimeout('GetDataINF()',1);</script>"));
				server1.sendContent(F("<title>Конфигурация OLED модуля</title><meta http-equiv='Content-Type' content='text/html; charset=utf-8'></head><body>"));
				server1.sendContent(F("<table><tr><td>Время включения дисплея</td><td>Время выключения дисплея</td></tr><tr><td><input required type='time' id='startD' onchange='sendTimeD()' /></td>  <td><input required type='time' id='stopD' onchange='sendTimeD()' /></td></tr><tr>Яркость дисплея <input required type='range' id='BR' min='0' max='255' onchange='sendTimeD()' /><td></td></tr></table>"));
			}
			else if(server1.arg("type") == "get"){
				String data = "";
				data += (String)Setting.StartDisplay.Hour + ";" + (String)Setting.StartDisplay.Minutes + ";" + (String)Setting.StopDisplay.Hour + ";" + (String)Setting.StopDisplay.Minutes + ";" + (String)Setting.DisplayBR + ";" + (String)MaximumPages + ";" + (String)MaxNamePage + ";" + (String)MaxPageValue + ";" + (String)maxpages + ";";
				server1.send(200, "text/html", data);
			}
			else if(server1.arg("type") == "set"){
				int StH = server1.arg("sth").toInt();
				int StM = server1.arg("stm").toInt();
				int SoH = server1.arg("soh").toInt();
				int SoM = server1.arg("som").toInt();
				int BR = server1.arg("br").toInt();
				
				Setting.StartDisplay.Hour = StH;
				Setting.StartDisplay.Minutes = StM;
				Setting.StopDisplay.Hour = SoH;
				Setting.StopDisplay.Minutes = SoM;
				Setting.DisplayBR = BR;
				Save();
				ChangeLight();
				server1.send(200, F("text/html"), F("OK"));
			}
			else if(server1.arg("type") == "setpage"){
				int pageID = server1.arg("id").toInt();
				CurrentPage = max(0, pageID);
				CurrentPage = min(CurrentPage, maxpages);
				UpdatePageData(Pages[CurrentPage]);
				UpdateData();
				rtos1.Zero();
				server1.send(200, F("text/html"), F("OK"));
			}
		});
		_UpdateLight.Start();	
		DisplayUpdate.Start();
		/*((*gtw).GetTelnet()).on("minusday","LoL", [&]() {
			(*gtw).PortPrint((String)getminusday());
		});*/		
	//#endif
}

void OLEDModule::parse(String datareads)
{
	int idexerror = datareads.lastIndexOf("error");
    if(idexerror != -1 || datareads.length() < 40){
	    (*gtw).AddLogFile(F("Data parsing error"));
		EtherFail = true;
		Attempted += 1;
        return;
    }
    
    DynamicJsonDocument root(2048);
	deserializeJson(root, datareads);

    Attempted = 0;

    if (!EtherFail) {
		maxpages = min(MaximumPages, static_cast<int>(root["pages"].size()));
		for (uint8_t i = 0; i < maxpages; i++) {
			memcpy(Pages[i].NamePage, root["pages"][i][0].as<const char*>(), MaxNamePage);
			memcpy(Pages[i].PageValue, root["pages"][i][1].as<const char*>(), MaxPageValue);
		}
    }
}

void OLEDModule::Save() {	
	File f = LittleFS.open(F("/OLEDModule.bin"), "w");
    if (!f) {
        Serial.println(F("Error open OLEDModule.bin for write"));  //  "открыть файл не удалось"
        return;
    }
    else{
        f.write((byte *)&Setting, sizeof(Setting));
		f.close();                    
        return;
    }
}

void OLEDModule::Load() {	
	OLEDSetting ss;
    
    File f = LittleFS.open(F("/OLEDModule.bin"), "r");
    if (!f) { // "открыть файл не удалось"
        ss.StartDisplay = (*gtw).GetDateTime();
        ss.StopDisplay = (DateTime){21, 0, 0, 0, 0, 0};
		Save();
		Setting = ss;
    }
    else{
        f.read((byte *)&ss, sizeof(ss));
		f.close();
		Serial.println(F("OLEDModule configuration file sucess reading"));  		
        Setting = ss;
    }
}

void OLEDModule::running()
{
	if((*gtw).IsSaveMode)
			return;
		
	if ((*gtw).GetSettingRead()) {
		rtos.ChangeTime((*gtw).GetSettingToInt("update", 300000));
		rtos1.ChangeTime((*gtw).GetSettingToInt("timeupdate", 4000));
		rtos.Zero();
		rtos1.Zero();
	}
	
	if(_UpdateLight.IsTick()) //На управление подствветкой
		ChangeLight();
			  
	if (rtos1.IsTick()){ //Смена страницы
		ChangePage();
	}
	  
	if (rtos.IsTick() && IsAutoUpdate){ //На обновление
		startUpdate();	
	}

	if(DisplayUpdate.IsTick()){
		UpdateData();
	}
}

void OLEDModule::startUpdate(){
	pause = false;
    parse((*gtw).SendToGateway((String)PathToPrognoz + (String)"&module=" + (*gtw).GetModule(0)));	
}