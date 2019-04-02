// 
// 
// 

#include "RemoteControl.h"

RemoteControl::RemoteControl(uint8_t pin,uint8_t beep_pin)
{
    _ir_pin = pin;
    _beep_pin = beep_pin;
    pinMode(beep_pin,OUTPUT);
    digitalWrite(beep_pin,HIGH);
    recv = new IRrecv(pin);
   
}

void RemoteControl::init()
{
    recv->enableIRIn();
     lcd.begin();
    lcd.print("Loading...");
    editMode = false;
    
}

void RemoteControl::attachOnInvalidCode(void (* callback)())
{
    onInvalidCode = callback;
}

void RemoteControl::updateDisplay()
{
    if(!editMode) lcd.noCursor();
    lcd.setCursor(0,0);
    if (current_screen==HOME_SCREEN){
        lcd.print(F("  -=GUARDUINO=- "));
        lcd.setCursor(0,1);
        lcd.print(F("  HOME SECURITY "));
        return;
    }
    if(current_screen == DETECTION_TIMEOUT_SCREEN){
        lcd.print(F("MOTION TIMEOUT: "));
        lcd.setCursor(0,1);
        lcd.print(F("                "));
        lcd.setCursor(0,1);
        lcd.print(Settings.Current.AlarmTimeout);
        if(!editMode) lcd.print(" Seconds");
        return;
    }
    if(current_screen ==PIN_SCREEN){
        lcd.print(F("UNLOCK PIN CODE:"));
        lcd.setCursor(0,1);
        lcd.print(F("                "));
        lcd.setCursor(0,1);
        int len = strlen(Settings.Current.Code);
        for(int i = 0;i<len;i++)
        {
            lcd.print(F("*"));
        }
        return;
    }
    if(current_screen == CHANGE_PASSWORD_SCREEN){
        lcd.print(F("MASTER PASSWORD:"));
        lcd.setCursor(0,1);
        lcd.print(F("                "));
        lcd.setCursor(0,1);
        int l = strlen(Settings.Current.Password);
        for(int i = 0;i<l;i++)
        {
            lcd.print(F("*"));
        }
        return;
    }

    lcd.print(F("USER "));
        lcd.print(current_screen-PIN_SCREEN);
       lcd.print(F(" NUMBER:  "));
        lcd.setCursor(0,1);
        auto rcv = Settings.Current.Receivers[current_screen-ADMIN1_SCREEN];
        if(strlen(rcv)==0 && !editMode)
        {
            lcd.print(F("[EMPTY]         "));
        } else {
            lcd.print(F("                "));
            lcd.setCursor(0,1);
            lcd.print(Settings.Current.Receivers[current_screen-ADMIN1_SCREEN]);
        }
}

void RemoteControl::saveBuffer()
{
    if(current_screen == HOME_SCREEN) return;
    if (current_screen==DETECTION_TIMEOUT_SCREEN)
        Settings.Current.AlarmTimeout = atoi(CodeBuffer);
    else if(current_screen == PIN_SCREEN)
        strcpy(Settings.Current.Code,CodeBuffer);
    else if(current_screen == CHANGE_PASSWORD_SCREEN)
        strcpy(Settings.Current.Password,CodeBuffer);
    else
        strcpy(Settings.Current.Receivers[current_screen-ADMIN1_SCREEN],CodeBuffer);
    
    Settings.SaveConfig();
    updateDisplay();
}

void RemoteControl::showLogin()
{
    cursor_column = 0;
    CodeIndex = 0;
    for (int i = 0; i < 47; ++i)
    {
        CodeBuffer[i] = 0;
    }
    current_screen = LOGIN_SCREEN;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("ENTER PASSWORD: "));
    lcd.setCursor(0,1);
    lcd.cursor();
}

void RemoteControl::onVolumeUp()
{
    if(current_screen == LOGIN_SCREEN)return;
     if(!isAuthenticated)
    {
        showLogin();
        return;
    }
    if(!editMode){
       
        current_screen++;
        if(current_screen>ADMIN7_SCREEN) current_screen = DETECTION_TIMEOUT_SCREEN;
        updateDisplay();
    } else
    {
       if(current_screen==DETECTION_TIMEOUT_SCREEN)
       {
           int value = atoi(CodeBuffer);
           value++;
           itoa(value,CodeBuffer,10);
           lcd.setCursor(0,1);
           lcd.print("                ");
           lcd.setCursor(0,1);
           lcd.print(CodeBuffer);
           CodeIndex = strlen(CodeBuffer);
           cursor_column = CodeIndex;
           lcd.setCursor(cursor_column,1);

       }
    }
}

void RemoteControl::onEQ()
{
    if(current_screen == LOGIN_SCREEN)
    {
        bool success = true;
        auto len = strlen(Settings.Current.Password);
        if (strlen(CodeBuffer) == len) {
            for (int i = 0; i < len; i++)
            {
                if (CodeBuffer[i] != Settings.Current.Password[i]) {
                    success = false;
                    break;
                }
            }
        }
        else
        {
            success = false;
        }

        for (int i = 0; i < 47; i++)
            CodeBuffer[i] = 0;

        isAuthenticated = success;
        if(success)
        {
            beep(444,74);
            current_screen = DETECTION_TIMEOUT_SCREEN;
            updateDisplay();
        } else
        {
            lcd.clear();
            lcd.print(F("INVALID PASSWORD"));
            lcd.setCursor(0,1);
            lcd.print(F("PLEASE TRY AGAIN"));

            beep(1111,74);
            showLogin();
        }

        return;
    }

    if(current_screen == HOME_SCREEN)
    {
        return;
    }
    if(editMode)
    {   
        editMode = false;
        saveBuffer();
        lcd.noCursor();
    } else {
        editMode = true;
        
        if (current_screen==DETECTION_TIMEOUT_SCREEN)
            itoa(Settings.Current.AlarmTimeout,CodeBuffer,10);
        else if(current_screen ==PIN_SCREEN)
            strcpy(CodeBuffer,Settings.Current.Code);
        else if(current_screen ==CHANGE_PASSWORD_SCREEN)
            strcpy(CodeBuffer,Settings.Current.Password);
        else
            strcpy(CodeBuffer,Settings.Current.Receivers[current_screen-ADMIN1_SCREEN]);
            
        updateDisplay();
        lcd.setCursor(0,1);
        lcd.cursor();
        cursor_column = 0;
        CodeIndex = 0;
        for (int i = 0; i < 47; i++)
            CodeBuffer[i] = 0;
    }
}

void RemoteControl::onVolumeDown()
{
    if(current_screen == LOGIN_SCREEN)return;
    if(!isAuthenticated)
        {
            showLogin();
            return;
        }

    if(editMode)
    {  
         
        if(current_screen==DETECTION_TIMEOUT_SCREEN)
       {
           int value = atoi(CodeBuffer);
           value--;
            if(value<0) value = 0;
           itoa(value,CodeBuffer,10);
            lcd.setCursor(0,1);
           lcd.print("                ");
           lcd.setCursor(0,1);
           lcd.print(CodeBuffer);
           CodeIndex = strlen(CodeBuffer);
           cursor_column = CodeIndex;
           lcd.setCursor(cursor_column,1);
       }
        
    } 
    else 
    {
        if(current_screen<=DETECTION_TIMEOUT_SCREEN)
        {
            current_screen = ADMIN7_SCREEN;
        } else {
            current_screen--;
        }
        updateDisplay();
    }
}

void RemoteControl::enable()
{
    isEnabled = true;
    lcd.setCursor(0,0);
    lcd.print(F(" ALARM ENABLED  "));
    lcd.setCursor(0,1);
    lcd.print(F("                "));
}

void RemoteControl::disable()
{
    isEnabled = false;
    lcd.setCursor(0,0);
    lcd.print(F(" ALARM DISABLED "));
    lcd.setCursor(0,1);
    lcd.print(F("                "));
}

void RemoteControl::lock()
{
    isLocked = true;
    lcd.setCursor(0,0);
    lcd.print(F(" SYSTEM LOCKED  "));
    lcd.setCursor(0,1);
    lcd.print(F(" ALARM ENABLED  "));
}

void RemoteControl::unlock()
{
    isLocked = false;
    lcd.setCursor(0,0);
    lcd.print(F("SYSTEM UNLOCKED "));
    lcd.setCursor(0,1);
    lcd.print(F(" REMOTE ENABLED "));
}


void RemoteControl::update()
{
    if(millis()-last_update_display > 7474)
    {
        last_update_display = millis();
        updateDisplay();
    }
    if (millis() - last_update < interval) return;
    last_update = millis();

    if (recv->decode(&results)) {
        if(isLocked)
        {
             recv->resume();
            return;
        }
        bool has_key = true;
        switch (results.value)
        {
        case KEY_CH:
        case KEY_CH_MINUS:
        case KEY_CH_PLUS:
            if (onLockKey) onLockKey();
            isAuthenticated = false;
            has_key = false;
            break;
        case KEY_PREV:
            if(!isEnabled){
                if(current_screen>=ADMIN1_SCREEN)
                {
                    if(!editMode){
                        Settings.Current.Receivers[current_screen-ADMIN1_SCREEN][0] = 0;
                        CodeBuffer[0] = 0;
                        CodeIndex = 0;
                        cursor_column = 0;
                        lcd.setCursor(0,1);
                        lcd.print(F("[EMPTY]         "));
                        lcd.noCursor();
                        editMode = false;
                        Settings.SaveConfig();
                    } else if(CodeIndex>0)
                    {
                        CodeIndex--;
                        CodeBuffer[CodeIndex] = 0;
                        cursor_column--;
                        lcd.setCursor(cursor_column,1);
                        lcd.print(" ");
                        lcd.setCursor(cursor_column,1);
                    }
                   
                } else if(current_screen == HOME_SCREEN)
                {
                }
                break;
            }
        case KEY_NEXT:
            if(!isEnabled)
            {
                break;
            }
        case KEY_PLAY:
            if(isEnabled)
            {
                CheckCode();
                has_key = false;
            } else
            {
                editMode = false;
                isAuthenticated = false;
                current_screen = HOME_SCREEN;
                updateDisplay();
            }
            break;
        case KEY_VOL_DOWN:
            if(!isEnabled){
                onVolumeDown();
            }
            break;
        case KEY_VOL_UP:
            if(!isEnabled){
                onVolumeUp();
            }
            break;
        case KEY_EQ:
            if(!isEnabled){
                onEQ();
            }
            break;
        case KEY_0:
            onKeyPress('0');
            break;
        case KEY_1:
            onKeyPress('1');
            break;
        case KEY_2:
            onKeyPress('2');
            break;
        case KEY_3:
            onKeyPress('3');
            break;
        case KEY_4:
            onKeyPress('4');
            break;
        case KEY_5:
            onKeyPress('5');
            break;
        case KEY_6:
            onKeyPress('6');
            break;
        case KEY_7:
            onKeyPress('7');
            break;
        case KEY_8:
            onKeyPress('8');
            break;
        case KEY_9:
            onKeyPress('9');
            break;
        default:
            has_key = false;
        }

        if(has_key)
        {
            beep();
        }
        

        recv->resume();
    }
}

void RemoteControl::beep(int d,int duration)
{
    for (int i = 0; i <duration; i++) 
    {
      digitalWrite (_beep_pin, LOW) ;
      delayMicroseconds (d) ;
      digitalWrite (_beep_pin, HIGH) ;
      delayMicroseconds (d) ;
    }
    digitalWrite(_beep_pin,HIGH);
}


void RemoteControl::CheckCode()
{
    if(!isEnabled) return;
    bool success = true;
    int len = strlen(Settings.Current.Code);
    if (strlen(CodeBuffer) == len) {
        for (int i = 0; i < len; i++)
        {
            if (CodeBuffer[i] != Settings.Current.Code[i]) {
                success = false;
                break;
            }
        }
    }
    else
    {
        success = false;
    }

    CodeIndex = 0;
    for (int i = 0; i < 47; i++)
        CodeBuffer[i] = 0;

    if (success) {
        {
            lcd.setCursor(0,0);
            lcd.print(F("  WELCOME HOME  "));
            lcd.setCursor(0,1);
            lcd.print(F(" ALARM DISABLED "));
            
            if(onUnlockKey) onUnlockKey();
        }
    } else if(!success && onInvalidCode)
    {
        lcd.setCursor(0,0);
        lcd.print(F("  INVALID CODE  "));
        lcd.setCursor(0,1);
        lcd.print(F("   TRY AGAIN!   "));
        beep(1111,74);
        onInvalidCode();
    }
}


void RemoteControl::attachOnLockKey(void(*cbk)())
{
    onLockKey = cbk;
}

void RemoteControl::attachOnUnlockKey(void(*cbk)())
{
    onUnlockKey = cbk;
}

//void RemoteControl::attachOnEQ(void(*callback)())
//{
//    onEQ = callback;
//}
//
//void RemoteControl::attachOnVolumeDown(void(*callback)())
//{
//    onVolumeDown = callback;
//}
//
//void RemoteControl::attachOnVolumeUp(void(*callback)())
//{
//    onVolumeUp = callback;
//}

void RemoteControl::onKeyPress(char number)
{

    if(current_screen == LOGIN_SCREEN)
    {
        lcd.print("*");
    } else {
        if(editMode)
        {
            if(current_screen == PIN_SCREEN)
            {
                lcd.setCursor(0,1);
                auto len = max(strlen(Settings.Current.Code), strlen(CodeBuffer));
                for(int i=0;i<len;i++)
                {
                    lcd.print(F("*"));
                }
                lcd.setCursor(cursor_column,1);
            } else if(current_screen == CHANGE_PASSWORD_SCREEN)
            {
                lcd.setCursor(0,1);
                auto len = max(strlen(Settings.Current.Password), strlen(CodeBuffer));
                for(int i=0;i<len;i++)
                {
                    lcd.print(F("*"));
                }
                lcd.setCursor(cursor_column,1);
            }
            lcd.print(number);
            
        }
    }
    CodeBuffer[CodeIndex] = number;
    CodeIndex++;
    cursor_column++;
    if (CodeIndex >= 47) CodeIndex = 0;
}
