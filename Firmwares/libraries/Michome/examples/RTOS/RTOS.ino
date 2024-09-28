#include <Michom.h>

RTOS rt(600000);

void setup(void){
    re.ChangeTime(600000);
}

void loop(void){
    if (rt.IsTick()) {
         //Код...
     }
}