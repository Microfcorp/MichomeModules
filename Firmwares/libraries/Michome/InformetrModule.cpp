#include "InformetrModule.h"

//
// конструктор - вызывается всегда при создании экземпляра класса InformetrModule
//
InformetrModule::InformetrModule(Michome *m)
{
    gtw = m;
	(*gtw).SetOptionFirmware(Infrometers, true);
}

void InformetrModule::init()
{
		(*gtw).preInit();
		Load();
	//#ifdef IsLCDI2C
		lcd.init();
		delay(10);
		lcd.noBacklight();
		lcd.setCursor(0, 0);
		lcd.print((*gtw).GetModule(0)); //ID
		lcd.setCursor(0, 1);
		lcd.print("Start module..."); //ID
		
		ESP8266WebServer& server1 = (*gtw).GetServer();
		server1.on("/onlight", [&]() {
			IsRunLight = true;
			LightOff.Start();
			lcd.backlight();
			server1.send(200, "text/html", "OK");
		});

		server1.on("/offlight", [&]() {
			IsRunLight = false;
			lcd.noBacklight();
			server1.send(200, "text/html", "OK");
		});

		server1.on("/refresh", [&]() {
			server1.send(200, "text/html", "OK");
			startUpdate();
		});

		server1.on("/pause", [&]() {
			if(server1.arg(0) == "stop")
			  pause = true;
			else if(server1.arg(0) == "run")
			  pause = false;
			else pause = !pause;
			server1.send(200, "text/html", (String)pause + " - OK");
		});

		server1.on("/print", [&]() {
			lcd.clear();
			lcd.home();
			lcd.setCursor(0, 0);
			lcd.print(server1.arg("fl"));
			lcd.setCursor(0, 1);
			lcd.print(server1.arg("sl"));
			server1.send(200, "text/html", String("OK"));
		});

		server1.on("/test", [&]() {
			lcd.noBacklight();
			lcd.backlight();
			ToPrognoz();

			lcd.clear();
			lcd.write(0);
			lcd.write(1);
			lcd.write(2);
			lcd.write(3);
			lcd.write(4);
			lcd.write(5);

			server1.send(200, "text/html", "System OK");
		});
		server1.on("/setdata", [&]() {
			parse(server1.arg(0));
			server1.send(200, "text/html", "OK");
		});
		server1.on("/inform", [&]() {
			if(server1.arg("type") == "show" || !server1.hasArg("type")){
				if (!server1.chunkedResponseModeStart(200, "text/html")) {
					server1.send(505, F("text/html"), F("HTTP1.1 required"));
					return;
				}
				server1.sendContent(F("<head>"));
				server1.sendContent(Styles);
				server1.sendContent(MainJS);
				server1.sendContent(AJAXJs);
				server1.sendContent(F("<script>function GetDataINF(){postAjax('/inform?type=get', GET, '', function(d){ab = d.split(';'); document.getElementById(\"startD\").value = (parseInt(ab[0]) < 10 ? '0' : '') + ab[0] + ':' + (parseInt(ab[1]) < 10 ? '0' : '') + ab[1]; document.getElementById(\"stopD\").value = (parseInt(ab[2]) < 10 ? '0' : '') + ab[2] + ':' + (parseInt(ab[3]) < 10 ? '0' : '') + ab[3];});} function sendTimeD(){var sth = document.getElementById(\"startD\").value.split(':')[0]; var stm = document.getElementById(\"startD\").value.split(':')[1]; var soh = document.getElementById(\"stopD\").value.split(':')[0]; var som = document.getElementById(\"stopD\").value.split(':')[1]; postAjax('/inform?type=set&sth='+sth+'&stm='+stm+'&soh='+soh+'&som='+som, GET, '', function(d){}); } window.setTimeout('GetDataINF()',1);</script>"));
				server1.sendContent(F("<title>Конфигурация информетра</title><meta http-equiv='Content-Type' content='text/html; charset=utf-8'></head><body>"));
				server1.sendContent(F("<table><tr><td>Время включения дисплея</td><td>Время выключения дисплея</td></tr>              <tr><td><input required type='time' id='startD' onchange='sendTimeD()' /></td>  <td><input required type='time' id='stopD' onchange='sendTimeD()' /></td></tr></table>"));
			}
			else if(server1.arg("type") == "get"){
				String data = "";
				data += (String)Setting.StartDisplay.Hour + ";" + (String)Setting.StartDisplay.Minutes + ";" + (String)Setting.StopDisplay.Hour + ";" + (String)Setting.StopDisplay.Minutes + ";";
				server1.send(200, "text/html", data);
			}
			else if(server1.arg("type") == "set"){
				int StH = server1.arg("sth").toInt();
				int StM = server1.arg("stm").toInt();
				int SoH = server1.arg("soh").toInt();
				int SoM = server1.arg("som").toInt();
				
				Setting.StartDisplay.Hour = StH;
				Setting.StartDisplay.Minutes = StM;
				Setting.StopDisplay.Hour = SoH;
				Setting.StopDisplay.Minutes = SoM;
				Save();
				server1.send(200, F("text/html"), F("OK"));
			}
		});
		UpdateLight.Start();	

		/*((*gtw).GetTelnet()).on("minusday","LoL", [&]() {
			(*gtw).PortPrint((String)getminusday());
		});*/		
	//#endif
}

void InformetrModule::parse(String datareads)
{
	int idexerror = datareads.lastIndexOf("error");
    if(idexerror != -1 || datareads.length() < 40){
		lcd.setCursor(0, 0);
        lcd.print("Error Update");
	    (*gtw).AddLogFile("Data parsing error");
		if(!IsReadData){
			WorkMode = Weather; //Это что бы включить часы, когда ошибка данных
			InverseWorkMode();
		}
        return;
    }
    
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(datareads);

    if (root["d"].as<int>() == 3) {
      Server.LightEnable = true;
      EtherFail = true;
      Attempted += 1;
    }
    else if (root["d"].as<int>() == 4) {
      Server.LightEnable = false;
      EtherFail = true;
      Attempted += 1;
    }
    else if (root["d"].as<int>() == 1) {
      Server.LightEnable = true;
      EtherFail = false;
      Attempted = 0;
    }
    else if (root["d"].as<int>() == 0) {
      Server.LightEnable = false;
      EtherFail = false;
      Attempted = 0;
    }
    else {
      EtherFail = true;
      Attempted += 1;
    }

    if (!EtherFail) {
	  maxday = min(DaysMaximum, static_cast<int>(root["data"].size()) - 1);
      for (int i = 0; i < maxday; i++) {
		memcpy(Server.Outdoor.Data[i].DayTemp, root["data"][i]["0"].as<char*>(), 6);
		memcpy(Server.Outdoor.Data[i].NightTemp, root["data"][i]["1"].as<char*>(), 6);
		memcpy(Server.Outdoor.Data[i].Wind, root["data"][i]["2"].as<char*>(), 6);
		memcpy(Server.Outdoor.Data[i].Press, root["data"][i]["3"].as<char*>(), 6);
		memcpy(Server.Outdoor.Data[i].ToDate, root["data"][i]["times"].as<char*>(), 16);
		Server.Outdoor.Data[i].UNIXDay = root["data"][i]["unixtime"].as<long>();
		Server.Outdoor.Data[i].IconDay = IDtoIcon(root["data"][i]["4"].as<byte>());
		Server.Outdoor.Data[i].IconNight = IDtoIcon(root["data"][i]["5"].as<byte>());
      }
	  memcpy(Server.Indoor.Data.DayTemp, root["temp"].as<char*>(), 6);
	  memcpy(Server.Indoor.Data.NightTemp, root["tempgr"].as<char*>(), 6);
	  memcpy(Server.Indoor.Data.Wind, root["hummgr"].as<char*>(), 6);
	  memcpy(Server.Indoor.Data.Press, root["dawlen"].as<char*>(), 6);
	  memcpy(Server.Indoor.Data.ToDate, root["time"].as<char*>(), 16);
	  Server.ServerTime = root["time"].as<String>();
	  //Server.Indoor.Data. = root["time"].as<String>() //<char*>()

      lcd.clear();
      lcd.home();
      lcd.setCursor(0, 0);
      lcd.print("Updating OK.");
	  (*gtw).PortPrintln("Updating data OK");
	  if(!IsReadData){
		WorkMode = Watch; //Это что бы включить погоду, когда данные были получены
		InverseWorkMode();
	  }
	  IsReadData = true;
	  rtos1.Stop();
	  rtos2.Start();
	  rtos2.Zero();
    }
}

void InformetrModule::parseH(String datareads)
{
	int idexerror = datareads.lastIndexOf("error");
    if(idexerror != -1 || datareads.length() < 40){
		lcd.setCursor(0, 0);
        lcd.print("Error Hourly");
	    (*gtw).AddLogFile("Data Hourly parsing error");
		IsHourly = false;
        return;
    }
    
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(datareads);    


	int maxhour = min(HoursMaximum, root["count"].as<int>());
	Server.Hourly.Count = maxhour;
	for (int i = 0; i < maxhour; i++) {
		memcpy(Server.Hourly.Data[i].Temp, root["data"][i]["0"].as<char*>(), 5);
		memcpy(Server.Hourly.Data[i].Time, root["data"][i]["times"].as<char*>(), 6);
		Server.Hourly.Data[i].Icon = IDtoIcon(root["data"][i]["1"].as<byte>());
	}
}

void InformetrModule::Save() {	
	File f = LittleFS.open("/informetr.bin", "w");
    if (!f) {
        Serial.println("Error open informetr.bin for write");  //  "открыть файл не удалось"
        return;
    }
    else{
        f.write((byte *)&Setting, sizeof(Setting));
		f.close();                    
        return;
    }
}

void InformetrModule::Load() {	
	InformetrSetting ss;
    
    File f = LittleFS.open("/informetr.bin", "r");
    if (!f) {
        ss.StartDisplay = (*gtw).GetDateTime();
        ss.StopDisplay = (DateTime){21, 0, 0, 0, 0, 0};
		Save();
		Setting = ss;
    }
    else{
        f.read((byte *)&ss, sizeof(ss));
		f.close();
		Serial.println("Informetr configuration file sucess reading");  //  "открыть файл не удалось"		
        Setting = ss;
    }
}

void InformetrModule::LoadHourlyPrognoz() {	
	if (!EtherFail && IsReadData){
		lcd.createChar(7, clockI);
		lcd.setCursor(0, 1);
		lcd.write(7);
		lcd.blink();
		parseH((*gtw).SendToGateway((String)PathToPrognoz + "&dt=" + (String)Server.Outdoor.Data[getminusday()].UNIXDay));
	}
}

void InformetrModule::running()
{
	if ((*gtw).GetSettingRead()) {
		rtos.ChangeTime((*gtw).GetSettingToInt("update", 600000));
		rtos2.ChangeTime((*gtw).GetSettingToInt("timeupdate", 4000));
		//rtos1.ChangeTime((*gtw).GetSettingToInt("timedate", pogr));

		/*if ((*gtw).GetSetting("running") == "0") 
		{
			rtos.Stop();
			rtos1.Stop();
			rtos2.Stop();
		}
		else 
		{
			rtos.Start();
			rtos1.Start();
			//rtos2.Start();
		}*/
	}
	

	if (rtos3.IsTick()) //На показ часов
		i3();
	if (rtos2.IsTick()) //На показ даты
		i2();		  
	if (rtos1.IsTick()) //На показ погоды
		i1();
	
	if(UpdateLight.IsTick())
		ChangeLight();
  
	if (rtos.IsTick() && IsAutoUpdate) //На обновление
      startUpdate();
	  
	if (LightOff.IsTick()) //На таймаут и отключение света
      IsRunLight = false;
}

void InformetrModule::printpause()
{
	if(!pause) return;
	lcd.setCursor(0, 1);
    lcd.print("P");
    lcd.blink();
}

void InformetrModule::nodat()
{
	lcd.clear();
	lcd.setCursor(0, 0);
    lcd.print((*gtw).GetModule(0));
	lcd.setCursor(2, 1);
    lcd.print("No data read");
}

void InformetrModule::PrintETError() {
	lcd.noBlink();
	lcd.clear();
	lcd.home();
	lcd.setCursor(0, 0);
	lcd.print("Ethernet Error");
	lcd.setCursor(0, 1);
	lcd.print("Attempt " + String(Attempted));
}

void InformetrModule::i1() { //Это показ значений
  rtos1.Stop();
  if(WorkMode == Weather){
	  if(pause){ 
		  printpause();
	  }	
	  else{
		  if(!IsReadData){
			nodat();
			return;
		  }
			
		  if (EtherFail) {  //При успешном соединении      
			PrintETError();
			return;
		  }
		  
		  if(IsHourly){
			printLCD((LCDDataPrint){BHourly, Server.Outdoor.Data[day]});  
		  }
		  else if (PokazType) {
			printLCD((LCDDataPrint){BOutDoor, Server.Outdoor.Data[day]});
			plusday();
		  }
		  else {
			printLCD((LCDDataPrint){BIndoor, Server.Indoor.Data});
		  }
	  } 
  }
  rtos2.Start();
  rtos2.Zero();
}

void InformetrModule::i2() { //Это показ даты
  rtos2.Stop();
  if(pause){ 
	  printpause();
  }
  else{
	  if(!IsReadData){
		nodat();
		return;
	  }
		
	  if (EtherFail) {
		PrintETError();
		return;
	  }
	  
	  if (PokazType && !IsHourly && WorkMode == Weather) {
		printLCD((LCDDataPrint){Day, Server.Outdoor.Data[day]});
	  }
  } 
  rtos1.Start();
  rtos1.Zero();
}

void InformetrModule::i3() { //Это показ даты
	if (WorkMode == Watch) {
		printLCD((LCDDataPrint){Watcher, Server.Indoor.Data});
	}
}

void InformetrModule::printLCD(LCDDataPrint dt)
{
	if(dt.Symbols != BHourly){
		lcd.clear();
		lcd.home();
	}
	if(dt.Symbols == BIndoor){
		lcd.noBlink();
		ToHomes();
		lcd.setCursor(0, 0);
		lcd.write(0);
		lcd.setCursor(2, 0);
		lcd.print(dt.Data.DayTemp);
		lcd.write(5);
		lcd.setCursor(9, 0);
		lcd.print(dt.Data.Wind);
		  
		lcd.setCursor(0, 1);
		lcd.write(0);
		lcd.setCursor(2, 1);
		lcd.print(dt.Data.NightTemp);
		lcd.write(5);
		lcd.setCursor(9, 1);
		lcd.print((String)dt.Data.Press);
	}
	else if(dt.Symbols == BOutDoor){
		lcd.noBlink();
		ToPrognoz();
		lcd.noBlink();
		lcd.setCursor(0, 0);
		lcd.write(dt.Data.IconDay);
		lcd.setCursor(0, 1);
		lcd.write(dt.Data.IconNight);
		lcd.setCursor(2, 0);
		lcd.print(dt.Data.DayTemp);
		lcd.write(5);
		lcd.setCursor(2, 1);
		lcd.print(dt.Data.NightTemp);
		lcd.write(5);
		lcd.setCursor(9, 0);
		lcd.print((String)dt.Data.Wind + "m/s");
		lcd.setCursor(9, 1);
		lcd.print((String)dt.Data.Press + "mm");
	}
	else if(dt.Symbols == BHourly){ //MaxSelH штуки
		lcd.noBlink();
		ToHourly();
		GenerateSelHours();
		for(int i = 0; i < MaxSelH; i++){			
			lcd.setCursor(i*4, 0);
			lcd.write(Server.Hourly.Data[selhours[i]].Icon);
			lcd.write(Server.Hourly.Data[selhours[i]].Temp[0]);
			lcd.write(Server.Hourly.Data[selhours[i]].Temp[1]);
			lcd.write(5);
			lcd.setCursor(i*4, 1);
			lcd.write(7);
			lcd.write(Server.Hourly.Data[selhours[i]].Time[0]);
			lcd.write(Server.Hourly.Data[selhours[i]].Time[1]);
			lcd.write(5);
		}
				
	}
	else if(dt.Symbols == Watcher){ //MaxSelH штуки
		lcd.noBlink();
		//ToWatch();
		lcd.setCursor(3, 0);	
		lcd.print((*gtw).GetFormattedDate());	
		lcd.setCursor(4, 1);	
		lcd.print((*gtw).GetFormattedTime());	
	}
	else if(dt.Symbols == Day){	
		lcd.setCursor(0, 1);
		PrintTextDayFromDate(dt.Data.ToDate);
        lcd.setCursor(0, 0);
        lcd.print(dt.Data.ToDate);		
        lcd.blink();
	}
}

void InformetrModule::startUpdate(){
	pause = false;
    lcd.clear();
    lcd.home();
    lcd.setCursor(0, 0);
    lcd.print("Start Update...");
	//(*gtw).AddLogFile("Start updating informetr");
	//String data = (*gtw).SendToGateway(PathToPrognoz);
	//Serial.println(data);
    parse((*gtw).SendToGateway(PathToPrognoz));	
}

//#ifdef IsLCDI2C
	void InformetrModule::SetDisplaySize(byte adress, byte x, byte y)
	{
		lcd = LiquidCrystal_I2C(adress, x, y);
		displayH = y;
		displayW = x;
	}
//#endif