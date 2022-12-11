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
}

void TimerLightModule::Load(){
    String rd = fstext.ReadFile();
    int countQ = (*gtw).Split(rd, '|', 0).toInt();
    String data = (*gtw).Split(rd, '|', 1);
    for(int i = 0; i < countQ; i++){
        String str = (*gtw).Split(data, '!', i);
        TimeLightModuleQ qq = {((byte)(*gtw).Split(str, ';', 0).toInt()), ((byte)(*gtw).Split(str, ';', 1).toInt()), ((byte)(*gtw).Split(str, ';', 2).toInt()), (*gtw).Split(str, ';', 3) == "1", ((byte)(*gtw).Split(str, ';', 4).toInt()), ((*gtw).Split(str, ';', 5).toInt()), ((*gtw).Split(str, ';', 6).toInt() == 1), ((*gtw).Split(str, ';', 7).toInt() == 1), ((*gtw).Split(str, ';', 8).toInt() == 1)};
        Qs.add(qq);
    } 

	rd = fsdays.ReadFile();
    countQ = (*gtw).Split(rd, '|', 0).toInt();
    data = (*gtw).Split(rd, '|', 1);
    for(int i = 0; i < countQ; i++){
        String str = (*gtw).Split(data, '!', i);
        DayStruct qq = {((byte)(*gtw).Split(str, ';', 0).toInt()), ((*gtw).Split(str, ';', 1) == "1"), ((byte)(*gtw).Split(str, ';', 2).toInt()), ((byte)(*gtw).Split(str, ';', 3).toInt()), ((*gtw).Split(str, ';', 4).toInt()) , ((*gtw).Split(str, ';', 5).toInt()) , ((*gtw).Split(str, ';', 6).toInt())};
        Ds.add(qq);
    } 	
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
		server1.sendContent(F("<title>Конфигурация таймеров</title><meta http-equiv='Content-Type' content='text/html; charset=utf-8'></head><body>"));
		#ifdef SendCurrentTime
			server1.sendContent(F("<p>Текущее время на модуле: <em><span id='timemod'>"));
			server1.sendContent((*gtw).GetFormattedTime());
			server1.sendContent(F("</span></em></p>"));
		#endif
		server1.sendContent(F("<p><a href='qsettings'>Настройка подсистемы точного времени модуля</a></p>"));
		server1.sendContent(F("<span style='color: #a52828;'><em>Обрабите внимание, что время таймера должно увеличиваться</em></span><br />"));
		#if defined(NoSaveHourAndMinutesForDayOFF)
			server1.sendContent(F("<span><em>При автоотключении таймера, его включение произойдет в 00:00 следующего дня</em></span><br />"));
		#else
			server1.sendContent(F("<span><em>При автоотключении таймера, его включение произойдет в тоже время отключения следующего дня</em></span><br />"));
		#endif
		server1.sendContent(F("<span><em>Используется таймеров "));
		server1.sendContent(String(Qs.size()));
		server1.sendContent(F("/"));
		server1.sendContent(String(MaximumTimers));
		server1.sendContent(F("</em></span><br />"));
		
		server1.sendContent(F("<a href='#' onclick='InvertVisible(document.getElementById(\"helpTimer\")); return;'>Описание работы таймеров</a>"));
		server1.sendContent(F("<div style='margin: 6px; display: none;' id='helpTimer'>\
			<span><em style='margin-left: 10px;'>Система таймеров работает по принципу последовательного выполнения действий сверху вниз. Пожалуйста, укажите время таймера, выберите исполняемый скрипт или вывод и укажите его значения.</em></span>\
			<br>\
			<span><em style='margin-left: 10px;'>Опция \"В диапазоне\", указывает на то, что данное действие необходимо выполнять каждую минуту до начала выполнения следующего скрипта</em></span>\
			<br>\
			<span><em style='margin-left: 10px;'>Опция \"Авто стоп\", указывает на то, что данное действие будет автоматически выключено на сутки, если что то изменило состояние указанного вывода. Не используется совместно со скриптом или эффектом. При изменении, все текущие отсчеты суток будут сброшены</em></span>\
			<br>\
			<span><em style='margin-left: 10px;'>Опция \"Авто возврат\", указывает на то, что данное действие будет автоматически включено, если измененное состояние вывода будет возвращено к изначальному значению</em></span>\
			<br>\
			<span><em style='margin-left: 10px;'>Подсвеченное действие обозначает, что текущее время находится в промежутке выполнения данного действия</em></span>\
			</div><br />"));
		server1.sendContent(F("<table style='width: fit-content;'><tbody>"));
		for(int i = 0; i < Qs.size(); i++){
            TimeLightModuleQ em = Qs.get(i);
			server1.sendContent(F("<tr "));
			server1.sendContent(IsPlayTimer(em) ? F("class='curtimer'") : F("class='nonetimer'"));
			server1.sendContent(F("><form action='/setqconfig'><input name='id' type='hidden' value='"));
			server1.sendContent((String)i);
			server1.sendContent(F("' /><td>Состояние: <input type='checkbox' "));
			server1.sendContent((em.Enable == 1 ? "checked": "class='nochecked'"));
			server1.sendContent(F(" name='en' /></td><td>Время: <input value='"));
			server1.sendContent((String)(em.Hour < 10 ? "0" : "")+em.Hour+":"+(em.Minutes < 10 ? "0" : "")+em.Minutes);
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
			server1.sendContent((String)MinimumBrightnes);
			server1.sendContent(F("' max='"));
			server1.sendContent((String)MaximumBrightnes);
			server1.sendContent(F("' name='state' value='"));
			server1.sendContent((String)em.State);
			server1.sendContent(F("' /> "));
			
			server1.sendContent(F("</div></td><td>В диапазоне: <input type='checkbox' "));
			server1.sendContent((em.IsDynamic ? "checked": "class='nochecked'"));
			server1.sendContent(F(" name='isdyn' /></td>"));
			
			server1.sendContent(F("<td>Авто стоп: <input type='checkbox'"));
			server1.sendContent((em.IsAutoOFF ? "checked": "class='nochecked'"));
			server1.sendContent(F(" name='isautooff' /></td>"));
			
			server1.sendContent(F("<td>Авто возврат: <input type='checkbox'"));
			server1.sendContent((em.IsAutoON ? "checked": "class='nochecked'"));
			server1.sendContent(F(" name='isautoon' /></td>"));
			
			server1.sendContent(F("<td><input type='submit' value='Сохранить' /></td><td><a href='/remove?id="));
			server1.sendContent((String)i);
			server1.sendContent(F("'>Удалить</a></td></form></tr>"));
        }
		server1.sendContent(F("<tr><td><a href='/addqtimer'>Добавить новый</a></td></tr></tbody></table><br /><a href='/'>Главная</a></body>"));server1.chunkedResponseFinalize();		
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
		
		(*light).StopAllEffect();
		
        Save();
        _running();
        server1.send(200, "text/html", "<head><meta charset=\"UTF-8\"><meta http-equiv='refresh' content='1;URL=/qconfig' /></head>Таймер №" + (String)id + " сохранен");
    });
    server1.on("/addqtimer", [&](){
        if(Qs.size() >= MaximumTimers){
            server1.send(200, "text/html", "<head><meta charset=\"UTF-8\"><meta http-equiv='refresh' content='1;URL=/qconfig' /></head><b>ОШИБКА!</b> Превышено максимальное число таймеров (" + (String)MaximumTimers + ")");
        }
        else{
            TimeLightModuleQ tm = {20, 02, 0, false, 0, 0, false, true, false};
            Add(tm);
            server1.send(200, "text/html", F("<head><meta charset=\"UTF-8\"><meta http-equiv='refresh' content='1;URL=/qconfig' /></head>Новый таймер успешно добавлен"));
        }
    });
    server1.on("/remove", [&](){       
        int id = server1.arg("id").toInt();
        Qs.remove(id);
        Save();
        server1.send(200, "text/html", "<head><meta charset=\"UTF-8\"><meta http-equiv='refresh' content='1;URL=/qconfig' /></head>Таймер №" + (String)id + " удален");
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
				if(em.IsAutoON && IsNeedEnableIsDay(i, pinID, State)){
					Change(i, true, false);
					RemoveChangeForDay(GetIDChangeForDay(i));
					isFind = true;
				}
				else if(em.IsAutoOFF){
					ChangeForDay(i, false, PreviewState, false, false);
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
}

bool TimerLightModule::IsNeedEnableIsDay(byte TLMid, int pin, int br){
    for(byte i = 0; i < Ds.size(); i++){
		DayStruct aq = Ds.get(i);
		if(aq.IdTLM == TLMid && aq.Pin == pin && aq.State == br){
			return true;
		}
	}
	return false;
}

void TimerLightModule::Change(byte id, bool state, bool isNeedSaveAndRun){
	TimeLightModuleQ em = Qs.get(id);
	em.Enable = state;
	Qs.set(id, em);
	
	if(isNeedSaveAndRun){
		Save();
		_running();
	}
}
//http://192.168.1.33/setqconfig?id=0&en=on&ctime=13%3A02&pin=0&state=1023&isdyn=on&isautooff=on
void TimerLightModule::ChangeForDay(byte id, bool state, int PinState, bool isRun, bool isSave){
    TimeLightModuleQ em = Qs.get(id);
	if(em.Enable == state) return;
	em.Enable = state;		
	Qs.set(id, em);
	//Save();	
	
	bool isfind = false;
	for(byte i = 0; i < Ds.size(); i++){
		DayStruct aq = Ds.get(i);
		if(aq.IdTLM == id){
			aq.IsCurrent = state;
			aq.ToOffDay = ((*gtw).GetDay() == 30 ? 1 : (*gtw).GetDay() + 1);
			#if defined(NoSaveHourAndMinutesForDayOFF)
				aq.ToOffHour = 0;
				aq.ToOffMinute = 0;
			#else
				aq.ToOffHour = (*gtw).GetHours();
				aq.ToOffMinute = (*gtw).GetMinutes();
			#endif
			aq.Pin = (!em.IsScript ? em.Pin : -1);
			aq.State = (!em.IsScript ? (PinState == -1 ? (*light).GetBrightness(em.Pin) : PinState) : -1);
			Ds.set(i, aq);
			isfind = true;
			break;
		}
	}
	if(!isfind){
		#if defined(NoSaveHourAndMinutesForDayOFF)
			DayStruct ds = {id, state, ((*gtw).GetDay() == 30 ? 1 : (*gtw).GetDay() + 1), 0, 0, (!em.IsScript ? em.Pin : -1), (!em.IsScript ? (PinState == -1 ? (*light).GetBrightness(em.Pin) : PinState) : -1)};
		#else
			DayStruct ds = {id, state, ((*gtw).GetDay() == 30 ? 1 : (*gtw).GetDay() + 1), (*gtw).GetHours(), (*gtw).GetMinutes(), (!em.IsScript ? em.Pin : -1), (!em.IsScript ? (PinState == -1 ? (*light).GetBrightness(em.Pin) : PinState) : -1)};
		#endif
		Ds.add(ds);
	}
	
	if(isSave)
		Save();
	if(isRun)
		_running();
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
    int Sled = -1;
	int i = GetIDTimer(em);
	TimeLightModuleQ Sledem;
	for(int a=i+1; a <= Qs.size(); a++){
		Sled = (a < Qs.size() ? a : -1);
		TimeLightModuleQ Sledem = Qs.get(Sled);
		if(Sledem.Enable) break;
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
				if(em.Pin < 50)
					(*light).RunScript(em.Pin);
				#ifdef EffectsON
				else
					(*light).RunEffect(em.Pin-50);
				#endif
			else
				(*light).AddBuferState(em.Pin, em.State);
		}
		
		/*int Sled = 0;
		TimeLightModuleQ Sledem;
		for(int a=i+1; a <= Qs.size(); a++){
			Sled = (a < Qs.size() ? a : -1);
			TimeLightModuleQ Sledem = Qs.get(Sled);
			if(Sledem.Enable) break;
		}
		//int Sled = (i+1 < Qs.size() ? i+1 : -1);
		//TimeLightModuleQ Sledem = Qs.get(Sled);
		
		if(em.Enable){
			if(!em.IsDynamic){
				if(em.Hour == (*gtw).GetHours() && em.Minutes == (*gtw).GetMinutes()){
					if(em.IsScript)
						(*light).RunScript(em.Pin);
					else
						(*light).AddBuferState(em.Pin, em.State);
				}
			}
			else{
				if(Sled != -1){
					if(em.Hour*60+em.Minutes <= (*gtw).GetHours()*60+(*gtw).GetMinutes() && Sledem.Hour*60+Sledem.Minutes >= (*gtw).GetHours()*60+(*gtw).GetMinutes()){
						if(em.IsScript)
							(*light).RunScript(em.Pin);
						else
							(*light).AddBuferState(em.Pin, em.State);
					}
				}
				else{
					if(em.Hour*60+em.Minutes <= (*gtw).GetHours()*60+(*gtw).GetMinutes()){
						if(em.IsScript)
							(*light).RunScript(em.Pin);
						else
							(*light).AddBuferState(em.Pin, em.State);
					}
				}
			}
		}*/
	}
    (*light).RunBuffer();
}