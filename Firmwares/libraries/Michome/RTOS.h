#ifndef RTOS_h
#define RTOS_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

#define RTOS24H (RTOS1H*24)
#define RTOS12H (RTOS1H*12)
#define RTOS3H (RTOS1H*3)
#define RTOS1H (RTOS10M*6)
#define RTOS10M (RTOS1M*10)
#define RTOS5M (RTOS1M*5)
#define RTOS3M (RTOS1M*3)
#define RTOS1M 60000
#define RTOS1S 1000
#define RTOS10S (RTOS1S*10)
#define RTOS30FPS 33
#define RTOS15FPS 66
#define RTOS5FPS 200
#define RTOS1FPS RTOS1S

class RTOS
{
        public:
                RTOS(long Time);
                RTOS();
                bool IsTick();
                bool IsRun();              
                void Stop();
                void Start();                
                void ChangeTime(long Time);
                void SetOffset(long Time);
				void Zero();
                long GetTime();
        private:
            long _Time = 0;
            long _previousMillis = 0;
            long _offvet = 0;
            long _offset_stop = 0;
            bool running;
};

#endif // #ifndef RTOS_h