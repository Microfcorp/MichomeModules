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
        sb += String(em.Hour) + ";" + String(em.Minutes) + ";" + String(em.Enable) + ";" + String(em.Pin) + ";" + String(em.State) + ";" + (em.IsDynamic ? "1" : "0") + "!";
    }                   
    fstext.WriteFile(sb);
	
	countQ = Ds.size();
    sb = ((String)countQ) + "|";
    for(byte i = 0; i < countQ; i++){
        DayStruct em = Ds.get(i);
        sb += String(em.IdTLM) + ";" + (em.IsCurrent ? "1" : "0") + ";" + String(em.ToOffDay) + ";" + String(em.ToOffHour) + ";" + String(em.ToOffMinute) + "!";
    }                   
    fsdays.WriteFile(sb);
}

void TimerLightModule::Load(){
    String rd = fstext.ReadFile();
    int countQ = (*gtw).Split(rd, '|', 0).toInt();
    String data = (*gtw).Split(rd, '|', 1);
    for(int i = 0; i < countQ; i++){
        String str = (*gtw).Split(data, '!', i);
        TimeLightModuleQ qq = {((byte)(*gtw).Split(str, ';', 0).toInt()), ((byte)(*gtw).Split(str, ';', 1).toInt()), ((byte)(*gtw).Split(str, ';', 2).toInt()), ((byte)(*gtw).Split(str, ';', 3).toInt()), ((*gtw).Split(str, ';', 4).toInt()), ((*gtw).Split(str, ';', 5).toInt() == 1)};
        Qs.add(qq);
    } 

	rd = fsdays.ReadFile();
    countQ = (*gtw).Split(rd, '|', 0).toInt();
    data = (*gtw).Split(rd, '|', 1);
    for(int i = 0; i < countQ; i++){
        String str = (*gtw).Split(data, '!', i);
        DayStruct qq = {((byte)(*gtw).Split(str, ';', 0).toInt()), ((*gtw).Split(str, ';', 1) == "1"), ((byte)(*gtw).Split(str, ';', 2).toInt()), ((byte)(*gtw).Split(str, ';', 3).toInt()), ((*gtw).Split(str, ';', 4).toInt())};
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
		server1.sendContent(AJAXJs+ChangeTypeJS);
		server1.sendContent(F("<title>Конфигурация таймеров</title><meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"));
		server1.sendContent(AutoChangeTime);
		server1.sendContent(F("</head><body><p>Время на модуле: <span id='timemod'>"));
		server1.sendContent((*gtw).GetFormattedTime());
		server1.sendContent(F("</span><br /><a href='qsettings'>Настройка системы таймеров</a></p><table><tbody>"));
		server1.sendContent(F("<span><em>Обрабите внимание, что время таймера должно увеличиваться</em></span>"));
		for(int i = 0; i < Qs.size(); i++){
            TimeLightModuleQ em = Qs.get(i);
			//server1.sendContent((String)"<tr><form action='/setqconfig'><input name='id' type='hidden' value='"+i+"' /><td>Состояние: <input type='checkbox' " + (em.Enable == 1 ? "checked": "") +" name='en' /></td><td>Время: <input value='"+(em.Hour < 10 ? "0" : "")+em.Hour+":"+(em.Minutes < 10 ? "0" : "")+em.Minutes+"' type='time' name='ctime' /></td><td>Пин: <select onload='changepin(this.value, "+i+")' onchange='changepin(this.value, "+i+")' name='pin'>"+GetPinsHTML(em.Pin)+"</select></td><td><span id='valuepin"+i+"'>Значение:</span> <input id='maxpin"+i+"' type='number' maxlength='4' min='"+MinimumBrightnes+"' max='"+MaximumBrightnes+"' name='state' value='"+em.State+"' /> " + "</td><td>В диапазоне: <input type='checkbox' " + (em.IsDynamic ? "checked": "") +" name='isdyn' /></td><td><input type='submit' value='Сохранить' /></td><td><a href='/remove?id="+i+"'>Удалить</a></td></form></tr>");
			server1.sendContent(F("<tr><form action='/setqconfig'><input name='id' type='hidden' value='"));
			server1.sendContent((String)i);
			server1.sendContent(F("' /><td>Состояние: <input type='checkbox' "));
			server1.sendContent((em.Enable == 1 ? "checked": "class='nochecked'"));
			server1.sendContent(F(" name='en' /></td><td>Время: <input value='"));
			server1.sendContent((String)(em.Hour < 10 ? "0" : "")+em.Hour+":"+(em.Minutes < 10 ? "0" : "")+em.Minutes);
			server1.sendContent(F("' type='time' name='ctime' /></td><td>Пин: <select onload='changepin(this.value, "));
			server1.sendContent((String)i);
			server1.sendContent(F(")' onchange='changepin(this.value, "));
			server1.sendContent((String)i);
			server1.sendContent(F(")' name='pin'>"));
			server1.sendContent(GetPinsHTML(em.Pin));
			server1.sendContent(F("</select></td><td><span id='valuepin"));
			server1.sendContent((String)i);
			server1.sendContent(F("'>Значение:</span> <input id='maxpin"));
			server1.sendContent((String)i);
			server1.sendContent(F("' type='number' maxlength='4' min='"));
			server1.sendContent((String)MinimumBrightnes);
			server1.sendContent(F("' max='"));
			server1.sendContent((String)MaximumBrightnes);
			server1.sendContent(F("' name='state' value='"));
			server1.sendContent((String)em.State);
			server1.sendContent(F("' /> "));
			server1.sendContent(F("</td><td>В диапазоне: <input type='checkbox' "));
			server1.sendContent((em.IsDynamic ? "checked": "class='nochecked'"));
			server1.sendContent(F(" name='isdyn' /></td><td><input type='submit' value='Сохранить' /></td><td><a href='/remove?id="));
			server1.sendContent((String)i);
			server1.sendContent(F("'>Удалить</a></td></form></tr>"));
        }
		server1.sendContent(F("<tr><td><a href='/addqtimer'>Добавить новый</a></td></tr></tbody></table><br /><a href='/'>Главная</a></body>"));		
    });
    server1.on("/setqconfig", [&](){
        int id = server1.arg("id").toInt();
        byte en = server1.arg("en") == "on"; 
        String times = server1.arg("ctime"); 
        byte hour = (*gtw).Split(times, ':', 0).toInt();
        byte minute = (*gtw).Split(times, ':', 1).toInt();
        byte pin = server1.arg("pin").toInt();
        int state = server1.arg("state").toInt();
        bool isdyn = server1.arg("isdyn") == "on";
        
        TimeLightModuleQ tm = {hour, minute, en, pin, state, isdyn};
        Qs.set(id, tm);
        Save();
        _running();
        server1.send(200, "text/html", "<head><meta charset=\"UTF-8\"><meta http-equiv='refresh' content='1;URL=/qconfig' /></head>Таймер №" + (String)id + " сохранен");
    });
    server1.on("/addqtimer", [&](){
        if(Qs.size() >= MaximumTimers){
            server1.send(200, "text/html", "<head><meta charset=\"UTF-8\"><meta http-equiv='refresh' content='1;URL=/qconfig' /></head><b>ОШИБКА!</b> Превышено максимальное число таймеров (" + (String)MaximumTimers + ")");
        }
        else{
            TimeLightModuleQ tm = {20, 02, 0, 0, MaximumBrightnes, false};
            Add(tm);
            server1.send(200, "text/html", F("<head><meta charset=\"UTF-8\"><meta http-equiv='refresh' content='1;URL=/qconfig' /></head>Новый таймер добавлен"));
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
        
    Load();
}

void TimerLightModule::Change(byte id, bool state){
	TimeLightModuleQ em = Qs.get(id);
	em.Enable = state;
	Qs.set(id, em);
	Save();
	_running();
}

void TimerLightModule::ChangeForDay(byte id, bool state){
    TimeLightModuleQ em = Qs.get(id);
	em.Enable = state;		
	Qs.set(id, em);
	Save();
	_running();
	
	bool isfind = false;
	for(byte i = 0; i < Ds.size(); i++){
		DayStruct aq = Ds.get(i);
		if(aq.IdTLM == id){
			aq.IsCurrent = state;
			aq.ToOffDay = ((*gtw).GetDay() == 30 ? 1 : (*gtw).GetDay() + 1);
			aq.ToOffHour = (*gtw).GetHours();
			aq.ToOffMinute = (*gtw).GetMinutes();
			Ds.set(i, aq);
			isfind = true;
			break;
		}
	}
	if(!isfind){
		DayStruct ds = {id, state, ((*gtw).GetDay() == 30 ? 1 : (*gtw).GetDay() + 1), (*gtw).GetHours(), (*gtw).GetMinutes()};
		Ds.add(ds);
	}
}

void TimerLightModule::Add(TimeLightModuleQ tm){
    Qs.add(tm);
    Save();
}

void TimerLightModule::running(){
    if (timers.IsTick()) {
        _running();
    }
}

void TimerLightModule::_running(){
	for(int i = 0; i < Ds.size(); i++){
		DayStruct em = Ds.get(i);
		if((*gtw).GetDay() == em.ToOffDay && (*gtw).GetHours() >= em.ToOffHour && (*gtw).GetMinutes() >= em.ToOffMinute || (*gtw).GetDay() > em.ToOffDay){
			TimeLightModuleQ am = Qs.get(em.IdTLM);
			am.Enable = !em.IsCurrent;
			Qs.set(em.IdTLM, am);
			Save();
		}
	}
	
	for(int i = 0; i < Qs.size(); i++){
		TimeLightModuleQ em = Qs.get(i);
		if(em.Enable){
			if(!em.IsDynamic){
				if(em.Hour == (*gtw).GetHours() && em.Minutes == (*gtw).GetMinutes()){
					(*light).AddBuferState(em.Pin, em.State);
				}
			}
			else{
				if(em.Hour*60+em.Minutes <= (*gtw).GetHours()*60+(*gtw).GetMinutes()){
					(*light).AddBuferState(em.Pin, em.State);
				}
			}
		}
	}
    (*light).RunBuffer();
}