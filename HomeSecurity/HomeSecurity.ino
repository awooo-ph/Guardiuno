#include "HomeSecurity.h"

// Define pins
#define BUZZER_PIN A3
#define BEEP_PIN A2
#define SENSOR_PIN 2
#define SENSOR2_PIN 3
#define KEY_PIN 5
#define IR_PIN 6
#define RESET_PIN 7

// Create RemoteControl instance
RemoteControl rc(IR_PIN,BEEP_PIN);

// Create MotionSensor instance
MotionSensor sensor(SENSOR_PIN, SENSOR2_PIN);

// Create SmsClass instance
SmsClass sms;

bool Locked = false;
bool AlarmEngaged = false;

void onLock()
{
    rc.enable();
    sensor.reset();    
    
    Locked = true;

    // beep
    digitalWrite(BUZZER_PIN,HIGH);
    delay(77);
    digitalWrite(BUZZER_PIN,LOW);
    delay(77);
    digitalWrite(BUZZER_PIN,HIGH);
}

void onUnlock()
{
    rc.disable();
    Locked = false;
    AlarmEngaged = false;

    // beep beep
    digitalWrite(BUZZER_PIN,LOW);
    delay(74);
    digitalWrite(BUZZER_PIN,HIGH);
    delay(111);
    digitalWrite(BUZZER_PIN,LOW);
    delay(74);
    digitalWrite(BUZZER_PIN,HIGH);
}

void onMovement()
{
    AlarmEngaged = true;

    // Turn buzzer on
    digitalWrite(BUZZER_PIN,LOW);

    // Send message to receivers
    for (auto number : Settings.Current.Receivers)
        sms.send(number,"INTRUDER ALERT!");
}

void onInvalidCode()
{
    // Send message to receivers
    for (auto r : Settings.Current.Receivers)
    {
        sms.send(r,"INVALID UNLOCK CODE ENTERED");
    }
}

void onSignalChange(int signal)
{
    // Update screen only when in HOME_SCREEN
    if(rc.current_screen == HOME_SCREEN)
    {
        rc.lcd.clear();
        if(signal>0){
            rc.lcd.print("SIGNAL: ");
            rc.lcd.print(signal);
        } else
        {
            rc.lcd.print("NO SIGNAL");
        }

        rc.lcd.setCursor(0,1);
        if(sms.isRegistered())
            rc.lcd.print(F("CONNECTED       "));
        else
            rc.lcd.print(F("CONNECTING...   "));
    }
}

void onMessageSent()
{
    // Update screen only when in HOME_SCREEN
    if(rc.current_screen != HOME_SCREEN) return;
    rc.lcd.clear();
    if(sms.getSignal()>0){
        rc.lcd.print("SIGNAL: ");
        rc.lcd.print(sms.getSignal());
    } else
    {
        rc.lcd.print("NO SIGNAL");
    }

    rc.lcd.setCursor(0,1);
    rc.lcd.print(F("MESSAGE SENT!   "));
}

byte buzz_state = LOW;
unsigned long last_buzz = 0;

// toot toot
void Buzz()
{
    // Don't buzz when alarm has started
    if(AlarmEngaged) return;
    if(sensor.hasMotion()) {
        if(buzz_state == LOW){
            if(millis() - last_buzz >= 1000)
            {
                last_buzz = millis();
                digitalWrite(BUZZER_PIN,LOW);
                buzz_state = HIGH;
            }
        } else
        {
            if(millis() - last_buzz >= 74)
            {
                digitalWrite(BUZZER_PIN,HIGH);
                buzz_state = LOW;
            }
        }
    } else
    {
        digitalWrite(BUZZER_PIN,HIGH);
        buzz_state = LOW;
    }
}

unsigned long last_checked = 0;
bool KeyLocked = false;
unsigned long last_key_changed = 0;
unsigned long last_key_check = 0;

// Checks the key switch state
void checkKey()
{
    // If the arduino has just started, don't check
    if(millis()<7777) return;

    // Check once per second
    if(millis()-last_key_check<1111) return;
    last_key_check = millis();

    // If KEY_PIN is HIGH, lock the system
    if(digitalRead(KEY_PIN))
    {
        if(!KeyLocked)
        {
            // debounce check
            if(millis()-last_key_changed<111) return;
            last_key_changed = millis();

            // call onLock only when the key state was previously LOW (unlocked)
            onLock();
        }

        // lock the remote control
        rc.lock();

        // set locked flag
        KeyLocked = true;
    } else
    {
        if(KeyLocked)
        {
            // debounce check
            if(millis()-last_key_changed<111) return;
            last_key_changed = millis();

            // call unlock on when key was previously HIGH (locked)
            onUnlock();
            rc.unlock();
        }

        KeyLocked = false;
    }
}

void setup() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN,HIGH);

    pinMode(KEY_PIN, INPUT_PULLUP);

    // Load settings from EEPROM
    Settings.LoadConfig();

    // Initialize RemoteControl instance
    rc.init();  

    // attach callback functions
    sensor.attachOnMovement(onMovement);
    sms.onMessageSent(onMessageSent);
    sms.onSignalChanged(onSignalChange);
    rc.attachOnLockKey(onLock);
    rc.attachOnUnlockKey(onUnlock);
    rc.attachOnInvalidCode(onInvalidCode);

    // Initialize class instances
    sms.init();
    sms.update();
    rc.updateDisplay();
}

void loop() {
    checkKey();    
    rc.update();
    sms.update();

    // update sensor state only when system is locked
    if(Locked) {
        Buzz();
        sensor.update();
    }
}