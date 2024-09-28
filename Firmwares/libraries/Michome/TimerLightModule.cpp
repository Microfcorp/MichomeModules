#include "TimerLightModule.h"

TimerLightModule::TimerLightModule(LightModules *m){
    light = m;
    gtw = &(*light).GetMichome();
	(*gtw).SetOptionFirmware(TimerLightModules, true);
	telnet = &(*gtw).GetTelnet();
	udp = &(*gtw).GetUDP();
}

void TimerLightModule::Save(){
    byte countQ = Qs.size();
    String sb = ((String)countQ) + "|";
    for(byte i = 0; i < countQ; i++){
        TimeLightModuleQ em = Qs.get(i);
        sb += String(em.Hour) + ";" + String(em.Minutes) + ";" + String(em.Enable) + ";" + String(em.IsScript ? "1" : "0") + ";" + String(em.Pin) + ";" + String(em.State) + ";" + (em.IsDynamic ? "1" : "0") + ";" + (em.IsAutoOFF ? "1" : "0") + ";" + (em.IsAutoON ? "1" : "0") + "!";
    }                   
    fstext.WriteFile(sb);
	
	countQ = Ds.size();
    sb = ((String)countQ) + "|";
    for(byte i = 0; i < countQ; i++){
        DayStruct em = Ds.get(i);
        sb += String(em.IdTLM) + ";" + (em.IsCurrent ? "1" : "0") + ";" + String(em.ToOffDay) + ";" + String(em.ToOffHour) + ";" + String(em.ToOffMinute) + ";" + String(em.Pin) + ";" + String(em.State) + "!";
    }                   
    fsdays.WriteFile(sb);
	
	File f = SFS.open(F("/TimerLightModule.bin"), "w");
    if (!f) {
        Serial.println(F("Error open TimerLightModule.bin for write"));  //  "открыть файл не удалось"
        return;
    }
    else{
        f.write((byte *)&M_settings, sizeof(M_settings));
		f.close();                    
        return;
    }
}

void TimerLightModule::Load(){
	Qs.clear();
    String rd = fstext.ReadFile();
    int countQ = (*gtw).Split(rd, '|', 0).toInt();
    String data = (*gtw).Split(rd, '|', 1);
    for(int i = 0; i < countQ; i++){
        String str = (*gtw).Split(data, '!', i);
        TimeLightModuleQ qq = {((byte)(*gtw).Split(str, ';', 0).toInt()), ((byte)(*gtw).Split(str, ';', 1).toInt()), ((byte)(*gtw).Split(str, ';', 2).toInt()), (*gtw).Split(str, ';', 3) == "1", ((byte)(*gtw).Split(str, ';', 4).toInt()), ((*gtw).Split(str, ';', 5).toInt()), ((*gtw).Split(str, ';', 6).toInt() == 1), ((*gtw).Split(str, ';', 7).toInt() == 1), ((*gtw).Split(str, ';', 8).toInt() == 1)};
        Qs.add(qq);
    } 

	Ds.clear();
	rd = fsdays.ReadFile();
    countQ = (*gtw).Split(rd, '|', 0).toInt();
    data = (*gtw).Split(rd, '|', 1);
    for(int i = 0; i < countQ; i++){
        String str = (*gtw).Split(data, '!', i);
        DayStruct qq = {((byte)(*gtw).Split(str, ';', 0).toInt()), ((*gtw).Split(str, ';', 1) == "1"), ((byte)(*gtw).Split(str, ';', 2).toInt()), ((byte)(*gtw).Split(str, ';', 3).toInt()), ((*gtw).Split(str, ';', 4).toInt()) , ((*gtw).Split(str, ';', 5).toInt()) , ((*gtw).Split(str, ';', 6).toInt())};
        Ds.add(qq);
    }

	TimerLightModuleSettings ss;  
    File f = LittleFS.open(F("/TimerLightModule.bin"), "r");
    if (!f) {
        ss.IsSync = false;
		memset(M_settings.SyncPath, 0x00, MaxSyncPath);
		M_settings = ss;
    }
    else{
        f.read((byte *)&ss, sizeof(ss));
		f.close();
		Serial.println(F("TimerLightModule configuration file sucess reading"));  //  "открыть файл не удалось"		
        M_settings = ss;
    } 

	if(M_settings.IsSync)
		timers_sync.Start();
	else
		timers_sync.Stop();
}

void TimerLightModule::init(){
	(*gtw).preInit();
    ESP8266WebServer& server1 = (*gtw).GetServer();
    
    server1.on("/qconfig", [&](){
		if (!server1.chunkedResponseModeStart(200, "text/html")) {
			server1.send(505, F("text/html"), F("HTTP1.1 required"));
			return;
		}
		server1.sendContent(F("<head>"));
		server1.sendContent(Styles);
		server1.sendContent(MainJS);
		server1.sendContent(AJAXJs);
		server1.sendContent(ChangeTypeJS);
		#ifdef SendCurrentTime
			server1.sendContent(AutoChangeTime);
		#endif
		server1.sendContent(F("<script>var defSync = "));
		server1.sendContent((M_settings.IsSync ? "true;" : "false;"));
		server1.sendContent(F("</script>"));
		server1.sendContent(F("<title>Конфигурация таймеров</title><meta http-equiv='Content-Type' content='text/html; charset=utf-8'></head><body onload=\"window.setTimeout('elementDisable(tabq, defSync);',50);\">"));
		#ifdef SendCurrentTime
			server1.sendContent(F("<p>Текущее время на модуле: <em><span id='timemod'>"));
			server1.sendContent((*gtw).GetFormattedTime());
			server1.sendContent(F("</span></em></p>"));
		#endif
		server1.sendContent(F("<p><a href='/timemodule?type=setting'>Настройка подсистемы точного времени модуля</a></p>"));
		server1.sendContent(F("<span style='color: #a52828;'><em>Обрабите внимание, что время таймера должно увеличиваться</em></span><br />"));
		#if defined(NoSaveHourAndMinutesForDayOFF)
			server1.sendContent(F("<span><em>При автоотключении таймера, его включение произойдет в 00:00 следующего дня</em></span><br />"));
		#else
			server1.sendContent(F("<span><em>При автоотключении таймера, его включение произойдет в тоже время отключения следующего дня</em></span><br />"));
		#endif
		server1.sendContent(F("<span><em>Используется таймеров <b>"));
		server1.sendContent(String(Qs.size()));
		server1.sendContent(F("/"));
		server1.sendContent(String(MaximumTimers));
		server1.sendContent(F("</b></em></span><br />"));
		
		server1.sendContent(F("<a href='#' onclick='InvertVisible(document.getElementById(\"helpTimer\")); return;'>Описание работы таймеров</a>"));
		server1.sendContent(F("<div style='margin: 6px; display: none;' id='helpTimer'>\
			<span><em style='margin-left: 10px;'>Система таймеров работает по принципу последовательного выполнения действий сверху вниз. Пожалуйста, укажите время таймера, выберите исполняемый скрипт или вывод управления и укажите его значение.</em></span>\
			<br>\
			<span><em style='margin-left: 10px;'>Опция \"В диапазоне\", указывает на то, что данное действие необходимо выполнять каждую минуту до начала выполнения следующего скрипта</em></span>\
			<br>\
			<span><em style='margin-left: 10px;'>Опция \"Авто стоп\", указывает на то, что данное действие будет автоматически выключено на сутки, если произошло внешнее изменение состояние указанного вывода. Не используется совместно со скриптом или эффектом. При изменении, все текущие остановки таймера будут сброшены</em></span>\
			<br>\
			<span><em style='margin-left: 10px;'>Опция \"Авто возврат\", указывает на то, что данное действие будет автоматически включено, если измененное состояние вывода было возвращено к изначальному значению</em></span>\
			<br>\
			<br>\
			<span><em style='margin-left: 10px; background-color: #f7e2e2;'>Подсвеченное данным цветом действие, обозначает, что текущее время находится в промежутке выполнения данного действия</em></span>\
			<br>\
			<span><em style='margin-left: 10px; background-color: #ffed0038;'>Подсвеченное данным цветом действие, обозначает, что таймер автоматически остановлен на сутки. Кнопка восстановить, автоматически вернет таймер в рабочее состояние</em></span>\
			</div><br />"));
		server1.sendContent(F("<table id='tabq' style='width: fit-content;'><tbody>"));
		for(int i = 0; i < Qs.size(); i++){
            TimeLightModuleQ em = Qs.get(i);
			LightPin lp = (*light).GetPin(em.Pin);
			server1.sendContent(F("<tr "));
			server1.sendContent(IsPlayTimer(em) ? F("class='curtimer'") : F("class='nonetimer'"));
			server1.sendContent(F("><form action='/setqconfig'><input name='id' type='hidden' value='"));
			server1.sendContent((String)i);
			server1.sendContent(F("' /><td "));
			server1.sendContent((em.Enable == 0 && IsDayOff(i) ? F("title='Автоматически выключен на сутки. Кнопка \"Восстановит\" текущий таймер и все, с ним связанные' class='isdayoff'") : F(" ")));
			server1.sendContent(F(">Состояние: <input type='checkbox' "));
			server1.sendContent((em.Enable == 1 ? F("checked") : F("class='nochecked'")));			
			server1.sendContent(F(" name='en' />"));
			server1.sendContent((em.Enable == 0 && IsDayOff(i) ? " <em><a href='/stoptimersday?type=2&timerid="+String(i)+"'>Восстановить</a></em>" : (" ")));
			server1.sendContent(F("</td><td>Время: <input value='"));
			server1.sendContent((String)(em.Hour < 10 ? F("0") : F(""))+em.Hour+":"+(em.Minutes < 10 ? F("0") : F(""))+em.Minutes);
			server1.sendContent(F("' type='time' name='ctime' /></td><td>Пин (Скрипт): <select onload='changepin(this.value, "));
			server1.sendContent((String)i);
			server1.sendContent(F(")' onchange='changepin(this.value, "));
			server1.sendContent((String)i);
			server1.sendContent(F(")' name='pin'>"));
			server1.sendContent(GetPinsHTML(em.Pin, em.IsScript));//changer
			server1.sendContent(F("</select></td>"));
			server1.sendContent(F("<td><div id='changer"));
			server1.sendContent((String)i);
			server1.sendContent(F("'><span id='valuepin"));
			//server1.sendContent(F("</select></td><td><span id='valuepin"));
			server1.sendContent((String)i);
			server1.sendContent(F("'>"));
			
			server1.sendContent(F("Значение:</span> <input id='maxpin"));
			server1.sendContent((String)i);
			server1.sendContent(F("' type='number' maxlength='4' min='"));
			server1.sendContent((String)lp.MinBrightnes());
			server1.sendContent(F("' max='"));
			server1.sendContent((String)lp.MaxBrightnes());
			server1.sendContent(F("' name='state' value='"));
			server1.sendContent((String)em.State);
			server1.sendContent(F("' /> "));
			
			server1.sendContent(F("</div></td><td>В диапазоне: <input type='checkbox' "));
			server1.sendContent((em.IsDynamic ? F("checked") : F("class='nochecked'")));
			server1.sendContent(F(" name='isdyn' /></td>"));
			
			server1.sendContent(F("<td>Авто стоп: <input type='checkbox'"));
			server1.sendContent((em.IsAutoOFF ? F("checked") : F("class='nochecked'")));
			server1.sendContent(F(" name='isautooff' /></td>"));
			
			server1.sendContent(F("<td>Авто возврат: <input type='checkbox'"));
			server1.sendContent((em.IsAutoON ? F("checked") : F("class='nochecked'")));
			server1.sendContent(F(" name='isautoon' /></td>"));
			
			server1.sendContent(F("<td><input type='submit' value='Сохранить' /></td><td><a href='/remove?id="));
			server1.sendContent((String)i);
			server1.sendContent(F("'>Удалить</a></td></form></tr>"));
        }
		if(Qs.size() < MaximumTimers)
			server1.sendContent(F("<tr><td><a href='/addqtimer'>Добавить новый</a></td></tr>"));
		else
			server1.sendContent(F("<tr><td><a>Создано максимальное количество таймеров</a></td></tr>"));
		server1.sendContent(F("</tbody></table>"));
		if(Qs.size() > 0)
			server1.sendContent(F("<p><a href='/stoptimersday?type=1'>Авто стоп таймеров на сутки</a></p>"));
		server1.sendContent(F("<form action='/setsyncconfig'><p>Синхронизация таймеров: Состояние <input id='scheck' name='state' type='checkbox' "));
		server1.sendContent((M_settings.IsSync ? F("checked") : F("class='nochecked'")));
		server1.sendContent(F(" onchange=\"elementDisable(document.getElementById('tabq'), this.checked)\" /> Модуль для синхронизации: <input name='module' value="));
		server1.sendContent((String)"'" + (String)M_settings.SyncPath + (String)"'");
		server1.sendContent(F(" type='text' /> <input type='submit' value='Сохранить' /> </p></form>"));
		server1.sendContent(F("<p><a href='/'>Главная</a></p></body>"));
		server1.chunkedResponseFinalize();		
    });
    server1.on("/setqconfig", [&](){
        int id = server1.arg("id").toInt();
        byte en = server1.arg("en") == "on"; 
        String times = server1.arg("ctime"); 
        byte hour = (*gtw).Split(times, ':', 0).toInt();
        byte minute = (*gtw).Split(times, ':', 1).toInt();
		bool IsScript = false;
        String pin = server1.arg("pin");
		byte pinID = 0;
		if(IsStr(pin, "script")){
			IsScript = true;
			pinID = (*gtw).Split(pin, '_', 1).toInt();
		}
		else if(IsStr(pin, "ef")){
			IsScript = true;
			pinID = (*gtw).Split(pin, '_', 1).toInt() + EFOffset;
		}
		else
			pinID = pin.toInt();
			
        int state = server1.arg("state").toInt();
        bool isdyn = server1.arg("isdyn") == "on";
        bool isautof = server1.arg("isautooff") == "on";
        bool isauton = server1.arg("isautoon") == "on";
        
        TimeLightModuleQ tm = {hour, minute, en, IsScript, pinID, state, isdyn, isautof, isauton};
        Qs.set(id, tm);
		
		for(byte i = 0; i < Ds.size(); i++){
			DayStruct aq = Ds.get(i);
			if(aq.IdTLM == id){
				Ds.remove(i);
				break;
			}
		}
		
		#ifdef EffectsON
			(*light).StopAllEffect();
		#endif
		
        Save();
        _running();
		
		(*gtw).GetUDP().SendMulticast((String)"UpdateTimers-" + WiFi.localIP().toString());
		
        server1.send(200, F("text/html"), "<head><meta charset=\"UTF-8\"><meta http-equiv='refresh' content='1;URL=/qconfig' /></head>Таймер №" + (String)id + " сохранен");
    });
	server1.on("/setsyncconfig", [&](){
        bool enb = server1.arg("state") == "on"; 
        String mod = server1.arg("module"); 
      
		mod.toCharArray(M_settings.SyncPath, MaxSyncPath);
		M_settings.IsSync = enb;
		
		Save();
		if(enb)
			_synchroT();
        Save();		
		
        server1.send(200, F("text/html"), F("<head><meta charset=\"UTF-8\"><meta http-equiv='refresh' content='1;URL=/qconfig' /></head>Сохранено"));
    });
    server1.on("/addqtimer", [&](){
        if(Qs.size() >= MaximumTimers){
            server1.send(200, F("text/html"), "<head><meta charset=\"UTF-8\"><meta http-equiv='refresh' content='1;URL=/qconfig' /></head><b>ОШИБКА!</b> Превышено максимальное число таймеров (" + (String)MaximumTimers + ")");
        }
        else{
            TimeLightModuleQ tm = {20, 02, 0, false, 0, 0, false, true, false};
            Add(tm);
            server1.send(200, F("text/html"), F("<head><meta charset=\"UTF-8\"><meta http-equiv='refresh' content='1;URL=/qconfig' /></head>Новый таймер успешно добавлен"));
        }
    });
    server1.on("/remove", [&](){       
        int id = server1.arg("id").toInt();
        Qs.remove(id);
        Save();
        server1.send(200, F("text/html"), "<head><meta charset=\"UTF-8\"><meta http-equiv='refresh' content='1;URL=/qconfig' /></head>Таймер №" + (String)id + " удален");
    });
	server1.on("/stoptimersday", [&](){       
		int types = server1.arg("type").toInt();
		if(types == 1){
			for(int i = 0; i < Qs.size(); i++){
				ChangeForDay(i, false, -1, false, false);			
			}
		}
		else if(types == 2){
			int timerid = server1.arg("timerid").toInt();
			//TimeLightModuleQ em = Qs.get(timerid);
			for(byte i = 0; i < Ds.size(); i++){
				DayStruct aq = Ds.get(i);
				if(aq.IdTLM == timerid){
					if(aq.State != -1 && aq.Pin != -1){
						(*light).ExternalSetLightID(aq.Pin, aq.State);
					}
					else{
						RemoveChangeForDay(i);
						Change(timerid, true, false);
					}
				}
			}
		}
		Save();
		_running();
        server1.send(200, F("text/html"), F("<head><meta charset=\"UTF-8\"><meta http-equiv='refresh' content='1;URL=/qconfig' /></head>OK"));
    });
	
	(*telnet).on("ontimer", "ontimer;timerid - Change state timer for on", [&](){
		String telnd = (*telnet).GetData();
		int t_id = (*gtw).Split(telnd, ';', 1).toInt();
		Change(t_id, true);
	});
	
	(*telnet).on("offtimer", "offtimer;timerid - Change state timer for off", [&](){
		String telnd = (*telnet).GetData();
		int t_id = (*gtw).Split(telnd, ';', 1).toInt();
		Change(t_id, false);
	});
	
	(*telnet).on("ontimerday", "ontimerday;timerid - Change state timer for on for one day", [&](){
		String telnd = (*telnet).GetData();
		int t_id = (*gtw).Split(telnd, ';', 1).toInt();
		ChangeForDay(t_id, true);
	});
	
	(*telnet).on("offtimerday", "offtimerday;timerid - Change state timer for off for one day", [&](){
		String telnd = (*telnet).GetData();
		int t_id = (*gtw).Split(telnd, ';', 1).toInt();
		ChangeForDay(t_id, false);
	});	
	
	(*udp).EventTimersData([&](IPAddress from, String cmd){
		String type = (*gtw).Split(cmd, ';', 0);
		int t_id = (*gtw).Split(cmd, ';', 1).toInt();
		
		if(type == "offtimerday")
			ChangeForDay(t_id, false);
		else if(type == "ontimerday")
			ChangeForDay(t_id, true);
		else if(type == "ontimer")
			Change(t_id, true);
		else if(type == "offtimer")
			Change(t_id, false);
	});
    
	(*light).OnExtPinChange([&](uint8_t pinID, int State, int PreviewState){
		bool isFind = false;
		for(int i = 0; i < Qs.size(); i++){
			TimeLightModuleQ em = Qs.get(i);
			if(!em.IsScript && em.Pin == pinID){
				if(em.IsAutoON && IsNeedEnableIsDay(i, pinID, State)){ //Возвращаем выключенный таймер
					Change(i, true, false);
					RemoveChangeForDay(GetIDChangeForDay(i));
					isFind = true;
				}
				else if(em.IsAutoOFF){ //Выключаем таймер
					bool changed = ChangeForDay(i, false, PreviewState, false, false);
					if(changed)
						isFind = true;
				}
			}
		}
		if(isFind){
			Save();
			_running();
		}
	});
    
    Load();
	
	(*gtw).OnFirstTimeSync([&](time_t unixTime){
		_running();
	});
	
	(*gtw).GetUDP().on("UpdateTimers", [&](IPAddress from, String cmd){
		if(cmd == M_settings.SyncPath)
			_synchroT();
	});
	
	if(M_settings.IsSync)
		_synchroT();
}

bool TimerLightModule::IsNeedEnableIsDay(byte TLMid, int pin, int br){ //Если надо вернуть состояние таймера
    for(byte i = 0; i < Ds.size(); i++){
		DayStruct aq = Ds.get(i);
		if(aq.IdTLM == TLMid && aq.Pin == pin && aq.State == br){
			return true;
		}
	}
	return false;
}

bool TimerLightModule::IsDayOff(byte TLMid){ //Если таймер выключен на сутки
    for(byte i = 0; i < Ds.size(); i++){
		DayStruct aq = Ds.get(i);
		if(aq.IdTLM == TLMid){
			return true;
		}
	}
	return false;
}

void TimerLightModule::Change(byte id, bool state, bool isNeedSaveAndRun){ //Изменить состояние таймера
	TimeLightModuleQ em = Qs.get(id);
	em.Enable = state;
	Qs.set(id, em);
	
	if(isNeedSaveAndRun){
		Save();
		_running();
	}
}

bool TimerLightModule::ChangeForDay(byte id, bool state, int PinState, bool isRun, bool isSave){
    TimeLightModuleQ em = Qs.get(id); //Получает объем таймера
	if(em.Enable == state) return false; //Если он уже нужного состояния, то возвращает 0
	em.Enable = state; //Изменяем значение	
	Qs.set(id, em); //Устанавливаем таймер
	//Save();	
	
	bool isfind = false;
	for(byte i = 0; i < Ds.size(); i++){
		DayStruct aq = Ds.get(i);
		if(aq.IdTLM == id){ //Выбираем для текущего таймера
			aq.IsCurrent = state; //Записываем текущее состояние
			aq.ToOffDay = ((*gtw).GetDay() == 30 ? 1 : (*gtw).GetDay() + 1); //Устанавливает день включения
			//Устанавливаем время включения
			#if defined(NoSaveHourAndMinutesForDayOFF)
				aq.ToOffHour = 0;
				aq.ToOffMinute = 0;
			#else
				aq.ToOffHour = (*gtw).GetHours();
				aq.ToOffMinute = (*gtw).GetMinutes();
			#endif
			aq.Pin = (!em.IsScript ? em.Pin : -1); //Устанавливаем пин
			aq.State = (!em.IsScript ? (PinState == -1 ? (*light).GetBrightness(em.Pin) : PinState) : -1); //Устанавливаем состояние для автовключения
			Ds.set(i, aq); //Записываем
			isfind = true; //Ставим флаг, что было изменение
			break;
		}
	}
	if(!isfind){ //Если не было изменений, то тобаляем новый
		#if defined(NoSaveHourAndMinutesForDayOFF)
			DayStruct ds = {id, state, ((*gtw).GetDay() == 30 ? 1 : (*gtw).GetDay() + 1), 0, 0, (!em.IsScript ? em.Pin : -1), (!em.IsScript ? (PinState == -1 ? (*light).GetBrightness(em.Pin) : PinState) : -1)};
		#else
			DayStruct ds = {id, state, ((*gtw).GetDay() == 30 ? 1 : (*gtw).GetDay() + 1), (*gtw).GetHours(), (*gtw).GetMinutes(), (!em.IsScript ? em.Pin : -1), (!em.IsScript ? (PinState == -1 ? (*light).GetBrightness(em.Pin) : PinState) : -1)};
		#endif
		Ds.add(ds);
	}
	
	if(isSave) //Если надо, то сохраняем
		Save();
	if(isRun) //Если надо, то применяем все значения
		_running();
	
	return true;
}

void TimerLightModule::RemoveChangeForDay(byte id){
    Ds.remove(id);
}

int TimerLightModule::GetIDChangeForDay(byte TLMid){
    for(byte i = 0; i < Ds.size(); i++){
		DayStruct aq = Ds.get(i);
		if(aq.IdTLM == TLMid){
			return i;
		}
	}
	return -1;
}

void TimerLightModule::Add(TimeLightModuleQ tm){
    Qs.add(tm);
    Save();
}

bool TimerLightModule::IsPlayTimer(TimeLightModuleQ em){
	ToCurrentPin.clear();
	for(int e = 0; e <= Qs.size(); e++){
		TimeLightModuleQ tmp = Qs.get(e);
		if((!tmp.IsScript && tmp.Pin == em.Pin) || tmp.IsScript)
			ToCurrentPin.add(tmp);
	}
	
    int Sled = -1;
	int i = GetIDTimer(em);
	TimeLightModuleQ Sledem;
	for(int a=i+1; a <= ToCurrentPin.size(); a++){
		Sled = (a < ToCurrentPin.size() ? a : -1);
		if(Sled != -1){
			Sledem = ToCurrentPin.get(Sled);
			if(Sledem.Enable) break;
		}
	}
	
	if(em.Enable){
		if(!em.IsDynamic){
			if(em.Hour == (*gtw).GetHours() && em.Minutes == (*gtw).GetMinutes()){
				return true;
			}
		}
		else{
			if(Sled != -1){
				if(em.Hour*60+em.Minutes <= (*gtw).GetHours()*60+(*gtw).GetMinutes() && Sledem.Hour*60+Sledem.Minutes > (*gtw).GetHours()*60+(*gtw).GetMinutes()){
					return true;
				}
			}
			else{
				if(em.Hour*60+em.Minutes <= (*gtw).GetHours()*60+(*gtw).GetMinutes()){
					return true;
				}
			}
		}
	}
	return false;
}

void TimerLightModule::running(){
    if (timers.IsTick()) {
        _running();
    }
	if (timers_sync.IsTick()){
		_synchroT();
	}
}

void TimerLightModule::_running(){	
	if(Ds.size() > 0){
		bool isNeedSave = false;
		for(int i = 0; i < Ds.size(); i++){
			DayStruct em = Ds.get(i);
			if(((*gtw).GetDay() == em.ToOffDay && (*gtw).GetHours() >= em.ToOffHour && (*gtw).GetMinutes() >= em.ToOffMinute) || (*gtw).GetDay() > em.ToOffDay){
				TimeLightModuleQ am = Qs.get(em.IdTLM);
				am.Enable = !em.IsCurrent;
				(*gtw).PortPrintln((String)"TLM " + (String)em.IdTLM + " is " + (am.Enable ? "on" : "off"));
				Qs.set(em.IdTLM, am);
				ToDeleteDS.add(i);
				isNeedSave = true;
			}
		}
		if(ToDeleteDS.size() > 0){
			for(int i = 0; i < ToDeleteDS.size(); i++){
				RemoveChangeForDay(ToDeleteDS.get(i));
			}	
			ToDeleteDS.clear();
		}
		
		if(isNeedSave)
			Save();
	}
	
	for(int i = 0; i < Qs.size(); i++){
		TimeLightModuleQ em = Qs.get(i);
		
		if(IsPlayTimer(em)){
			if(em.IsScript)
				if(em.Pin < EFOffset)
					(*light).RunScript(em.Pin);
				#ifdef EffectsON
				else
					(*light).RunEffect(em.Pin-EFOffset);
				#endif
			else
				(*light).AddBuferState(em.Pin, em.State);
		}
	}
	if((*light).IsBufferAvaliable())
		(*light).RunBuffer();
}

void TimerLightModule::_synchroT(){
	if(!M_settings.IsSync) return;
	
	String tim = (*gtw).SendDataGET("/fsedit?file=timer.txt", GetSyncHost(), GetSyncPort());
	String tday = (*gtw).SendDataGET("/fsedit?file=timerdays.txt", GetSyncHost(), GetSyncPort());
	
	if(CountSymbols(tim, '|') == 1){
		fstext.WriteFile(tim);
	}
	if(CountSymbols(tday, '|') == 1){
		fsdays.WriteFile(tday);
	}
	
	Load();
}