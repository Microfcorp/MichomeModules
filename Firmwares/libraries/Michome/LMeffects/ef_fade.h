#pragma once
#define Fade_Effect

// настройки перехода
#define USECRT 		    // Использовать CRT гамму
#define MIN_BRIGHT 300   // мин. яркость огня
#define MAX_BRIGHT MaximumBrightnes  // макс. яркость огня

#if defined(USECRT) && !defined(EnableCRT)
	#error "CRT гамма выключена в LightModule.h"
#endif

bool reversestate = false;
int val = 0;

void fadeTick(LinkedList<LightPin> *Pins) {
  if(reversestate) if(val-- <= MIN_BRIGHT) {reversestate = false;}
  if(!reversestate) if(val++ >= MAX_BRIGHT) {reversestate = true;}
  
  #if defined(USECRT)
	SetEveryPin(getBrightCRT(val));
  #else
	SetEveryPin(val);
  #endif
}