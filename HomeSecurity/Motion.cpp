// 
// 
// 

#include "Motion.h"

MotionSensor::MotionSensor(uint8_t pin,uint8_t pin2)
{
    _pin = pin;
    _pin2 = pin2;
    pinMode(pin2,INPUT);
    pinMode(pin,INPUT);
}

void MotionSensor::update()
{
    if(has_alarmed) return;
    if(digitalRead(_pin) || digitalRead(_pin2))
    {
        if(!has_movement)
            last_detection = millis();
        has_movement = true;
        last_movement = millis();
    }

    if(millis() - last_movement > 2222)
        has_movement = false;

    if(has_movement)
    {
        if(millis() - last_detection >= Settings.Current.AlarmTimeout*1000)
        {
            has_alarmed = true;
            if(onMovement) onMovement();
        }
    }
}

void MotionSensor::attachOnMovement(void (* callback)())
{
    onMovement = callback;
}


void MotionSensor::reset()
{
    has_movement = false;
    has_alarmed = false;
}
