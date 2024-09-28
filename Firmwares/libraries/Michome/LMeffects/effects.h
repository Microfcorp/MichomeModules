#pragma once
#ifndef LightModules_h
	#error "No LightModules.h file"
#endif
#ifndef EffectsON
	#define EffectsON //Эффекты включены
#endif

#include "ef_fire.h"
#include "ef_fade.h"

void LightModules::InitEffects()
{
	CreateEffect("Огонь", "Эффект огня", 0, fireTick);
	CreateEffect("Переход", "Эффект плавного затухания и нарастания", 4, fadeTick);
}