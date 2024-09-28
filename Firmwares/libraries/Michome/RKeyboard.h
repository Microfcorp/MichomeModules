#ifndef RKeyboard_h
#define RKeyboard_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

class RKeyboard
{
        public:
                //Объявление класса
                RKeyboard(uint8_t pin, int *structures, int numberkeys){RKeyboard::pin = pin; RKeyboard::structures = structures; RKeyboard::numberkeys = numberkeys*2;}
                //Текущее значение
                int CurrentValue(){return analogRead(RKeyboard::pin);}
                //Задать погрешность
                void SetMaxMin(int value){RKeyboard::pogr = value;}
                //Нажата ли клавиша
				bool PresedKey(int keynuber)
                {
                    int CValue = RKeyboard::CurrentValue();
                    for(uint8_t i=1; i < RKeyboard::numberkeys; i = i + 2){
                        int name = RKeyboard::structures[i-1];
                        int value = RKeyboard::structures[i];
                        if(name == keynuber){
                            if(value > (CValue - RKeyboard::pogr) & value < (CValue + RKeyboard::pogr))
                                return true;
                            else
                                return false;
                        }
                    }
                }
                //Какая клавиша нажата
                int PresedKey()
                {
                    int CValue = RKeyboard::CurrentValue();
                    for(uint8_t i=1; i < RKeyboard::numberkeys; i = i + 2){
                        int name = RKeyboard::structures[i-1];
                        int value = RKeyboard::structures[i];
                        if(value > (CValue - RKeyboard::pogr) & value < (CValue + RKeyboard::pogr))
                            return name;
                    }
                    return -1;
                }
                /*//Опрос клавиш
                void Tick()
                {
                    if(RKeyboard::rt.IsTick()){
                        int CValue = RKeyboard::CurrentValue();
                        for(uint8_t i=1; i < RKeyboard::numberkeys; i = i + 2){
                            int name = RKeyboard::structures[i-1];
                            int value = RKeyboard::structures[i];
                            if(value > (CValue - RKeyboard::pogr) & value < (CValue + RKeyboard::pogr))
                                RKeyboard::fl[i-1]++;
                        }
                    }
                }
                bool PresedKeyDouble(int keynuber){
                    int CValue = RKeyboard::CurrentValue();
                    for(uint8_t i=1; i < RKeyboard::numberkeys; i = i + 2){
                        int name = RKeyboard::structures[i-1];
                        int value = RKeyboard::structures[i];
                        if(name == keynuber){
                            if(value > (CValue - RKeyboard::pogr) & value < (CValue + RKeyboard::pogr))
                                if(RKeyboard::fl[i-1] == 2){return true; RKeyboard::fl[i-1] = 0;}
                                else{return false;}
                            else
                                return false;
                        }
                    }
                }*/
        private:
            uint8_t pin;
            int numberkeys;
            int pogr = 10;
            //int[][] structures = {0, 961, 1, 11};
            int *structures;
};
#endif // #ifndef Michom_h