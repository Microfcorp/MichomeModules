#ifndef ModuleTypes_h
#define ModuleTypes_h

#define StudioLight "StudioLight"
//#define Termometr "termometr" //Тип заменен на универсальный MeteoModule
#define Informetr "Informetr"
//#define Msinfoo "msinfoo"     //Тип заменен на универсальный MeteoModule
//#define HDC1080md "hdc1080mx" //Тип заменен на универсальный MeteoModule
#define WS28Module "WS28"         
#define MeteoModule "meteostation"
#define OLED "OLED"
#define PowerMods "PowerModules"

#define BUTTONPRESS "get_button_press"

/* static char* GenerateTypes(String arraystr[], byte size){
	volatile String localsq = "";
	for(byte i = 0; i < size; i++)
		localsq += arraystr[i] + "|";
	localsq = localsq.substring(0, localsq.length()-2);
    volatile char typs[localsq.length()];
    localsq.toCharArray(typs, localsq.length());
	return typs;
} */

#endif