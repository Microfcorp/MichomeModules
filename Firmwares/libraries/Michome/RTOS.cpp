#include "RTOS.h"

//
// конструктор - вызывается всегда при создании экземпляра класса RTOS
//
RTOS::RTOS(long Time)
{
    _Time = Time;
    running = true;
}

//
// конструктор - вызывается всегда при создании экземпляра класса RTOS
//
RTOS::RTOS()
{
    _Time = 1000;
    running = false;
}

void RTOS::ChangeTime(long Time)
{
    _Time = Time;
}

void RTOS::SetOffset(long Time)
{
    _offvet = Time;
}

void RTOS::Stop()
{
    running = false;
	_offset_stop = (millis() - _offvet) - _previousMillis;
}

void RTOS::Start()
{
    running = true;
	if(_offset_stop != 0)
		_previousMillis = millis() - _offset_stop;
	_offset_stop = 0;
}

void RTOS::Zero()
{
    _previousMillis = millis();   // запоминаем текущее время
}

bool RTOS::IsTick()
{
    if(!running)
        return false;
    
    if ((millis() - _offvet) - _previousMillis > _Time) {
       _previousMillis = millis();   // запоминаем текущее время
       return true;
    }
    else{
        return false;
    }
}

bool RTOS::IsRun()
{
    return running;   
}

long RTOS::GetTime()
{
    return _Time;   
}