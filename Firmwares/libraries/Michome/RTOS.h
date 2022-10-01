#ifndef RTOS_h
#define RTOS_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

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