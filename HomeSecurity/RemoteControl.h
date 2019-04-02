// RemoteControl.h

#ifndef _REMOTECONTROL_h
#define _REMOTECONTROL_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define KEY_CH_MINUS    0xFFA25D
#define KEY_CH          0xFF629D
#define KEY_CH_PLUS     0xFFE21D
#define KEY_PREV        0xFF22DD
#define KEY_NEXT        0xFF02FD
#define KEY_PLAY        0xFFC23D
#define KEY_VOL_DOWN    0xFFE01F
#define KEY_VOL_UP      0xFFA857
#define KEY_EQ          0xFF906F
#define KEY_0           0xFF6897
#define KEY_100         0xFF9867
#define KEY_200         0xFFB04F
#define KEY_1           0xFF30CF
#define KEY_2           0xFF18E7
#define KEY_3           0xFF7A85
#define KEY_4           0xFF10EF
#define KEY_5           0xFF38C7
#define KEY_6           0xFF5AA5
#define KEY_7           0xFF42BD
#define KEY_8           0xFF4AB5
#define KEY_9           0xFF52AD

//enum IR_KEYS
//{
//    ChannelDown = KEY_CH_MINUS,
//    Channel = KEY_CH      ,
//    ChannelUp = KEY_CH_PLUS ,
//    Previous = KEY_PREV    ,
//    Next = KEY_NEXT    ,
//    Play = KEY_PLAY    ,
//    VolumeDown = KEY_VOL_DOWN,
//    VolumeUp =KEY_VOL_UP  ,
//    EQ = KEY_EQ      ,
//    Num0 = KEY_0       ,
//    Plus100 = KEY_100     ,
//    Plus200 = KEY_200     ,
//    Key1 = KEY_1       ,
//    Key2 = KEY_2       ,
//    Key3 = KEY_3       ,
//    Key4 = KEY_4       ,
//    Key5 = KEY_5       ,
//    Key6 = KEY_6       ,
//    Key7 = KEY_7       ,
//    Key8 = KEY_8       ,
//    Key9 = KEY_9       ,
//};

#define HOME_SCREEN 0
#define LOGIN_SCREEN 1
#define DETECTION_TIMEOUT_SCREEN 2
#define CHANGE_PASSWORD_SCREEN 3
#define PIN_SCREEN 4
#define ADMIN1_SCREEN 5
#define ADMIN2_SCREEN 6
#define ADMIN3_SCREEN 7
#define ADMIN4_SCREEN 8
#define ADMIN5_SCREEN 9
#define ADMIN6_SCREEN 10
#define ADMIN7_SCREEN 11



#include <IRremote.h>
#include "Settings.h"

class RemoteControl
{
 private:
    byte _ir_pin = 7;
    byte _beep_pin = 0;
    IRrecv * recv;
    decode_results results;
    unsigned long last_update = 0;
    void (*onLockKey)() = nullptr;
    void (*onUnlockKey)() = nullptr;
    //void (*onVolumeUp)() = nullptr;
    //void (*onVolumeDown)() = nullptr;
    //void (*onEQ)() = nullptr;
    unsigned long last_update_display = 0;
    void (*onInvalidCode)() = nullptr;
    void onKeyPress(char);
    void CheckCode();
    char CodeBuffer[47] = {};
    int CodeIndex = 0;
    bool isEnabled = false;
    void beep(int d=444, int duration = 47);
    
    bool isAuthenticated = false;
    
    void onVolumeUp();
    void onVolumeDown();
    void onEQ();
    bool editMode=false;
    bool isLocked = false;
    byte cursor_column = 0;
    void saveBuffer();
    void showLogin();
 public:
    int current_screen = HOME_SCREEN;
    LiquidCrystal_I2C lcd;
    int interval = 100;
	RemoteControl(uint8_t pin, uint8_t beep_pin);
    void init();
    void update();
    void lock();
    void unlock();
    void updateDisplay();
    void attachOnInvalidCode(void (*callback)());
    void attachOnLockKey(void (*callback)());
    void attachOnUnlockKey(void (*callback)());
    //void attachOnVolumeUp(void (*callback)());
    //void attachOnVolumeDown(void (*callback)());
   // void attachOnEQ(void (*callback)());

    void enable();
    void disable();
};

#endif