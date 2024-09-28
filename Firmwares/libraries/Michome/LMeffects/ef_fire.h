#pragma once
#define Fire_Effect

// настройки пламени
#define HUE_START 0     // начальный цвет огня (0 красный, 80 зелёный, 140 молния, 190 розовый)
#define HUE_GAP 10      // коэффициент цвета огня (чем больше - тем дальше заброс по цвету)
#define SMOOTH_K 0.15    // коэффициент плавности огня (0.15)
#define MIN_BRIGHT 420   // мин. яркость огня
#define MAX_BRIGHT MaximumBrightnes  // макс. яркость огня
#define MIN_SAT 255     // мин. насыщенность
#define MAX_SAT 255     // макс. насыщенность
#define TIMEFIRE 50		// Скорость

uint32_t prevTime, prevTime2;
byte fireRnd = 0;
float fireValue = 0;

//void fireTick(LinkedList<LightPin> *Pins, ModuleParam *EFParams) {
void fireTick(LinkedList<LightPin> *Pins) {

  // задаём направление движения огня
  if (millis() - prevTime > 100) {
    prevTime = millis();
    fireRnd = random(0, 10);
  }
  // двигаем пламя
  if (millis() - prevTime2 > TIMEFIRE) {
    prevTime2 = millis();
    fireValue = (float)fireValue * (1 - SMOOTH_K) + (float)fireRnd * 10 * SMOOTH_K;
    /*strip.setHSV(
      constrain(map(fireValue, 20, 60, HUE_START, HUE_START + HUE_GAP), 0, 1023),   // H
      constrain(map(fireValue, 20, 60, MAX_SAT, MIN_SAT), 0, 1023),       // S
      constrain(map(fireValue, 20, 60, MIN_BRIGHT, MAX_BRIGHT), 0, 1023)  // V
    );*/
	SetEveryPin(constrain(map(fireValue, 20, 60, MIN_BRIGHT, MAX_BRIGHT), 0, 1000));
  }
}

ModuleParam* FireGetParams() {
	ModuleParam Ut[MaxEFParams];
	Ut[0] = {"SMOOTH_K", "0.25"};
	//Ut.add({"MIN_BRIGHT", "420"});
	//Ut.add({"TIMEFIRE", "30"});
	return Ut;
}