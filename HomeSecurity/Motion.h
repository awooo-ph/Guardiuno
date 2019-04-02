// Motion.h

#ifndef _MOTION_h
#define _MOTION_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Settings.h"

class MotionSensor
{
private:
    unsigned long last_detection = 0;
    bool has_movement = false;
    unsigned long last_movement = 0;
    uint8_t _pin;
    uint8_t _pin2;
    bool has_alarmed = false;
    void (*onMovement)() = nullptr;
    unsigned long last_update = 0;

 public:
   
    MotionSensor(uint8_t pin,uint8_t pin2);
	void update();
    void reset();
    bool hasMotion(){return has_movement;}
    bool hasAlarmed(){return has_alarmed;}
    void attachOnMovement(void (*callback)());
};

#endif

