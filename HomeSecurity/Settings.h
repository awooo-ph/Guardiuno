// Settings.h

#ifndef _SETTINGS_h
#define _SETTINGS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include <EEPROM.h>
#include "RemoteControl.h"

struct Config
{
    uint32_t LockKeys[3] = {KEY_CH,KEY_CH_PLUS,KEY_CH_MINUS};
    uint32_t UnlockKeys[3] = {KEY_PREV,KEY_NEXT,KEY_PLAY};
    char Code[47] = "777\0";
   // int CodeCount = 3;
    int AlarmTimeout = 7;
    char Password[47] = "777\0";
    char Receivers[7][15];
    unsigned int checksum;
};

class SettingsClass
{
private:


public:
    void SaveConfig();
    void ResetConfig();
    void LoadConfig();

    Config Current;
};

extern SettingsClass Settings;
#endif