#include "Sms.h"

#define DEBUG_SERIAL

//const char AT[]     PROGMEM = "AT\r";
//const char CMGD[]   PROGMEM = "AT+CMGD=1,1\r";
//const char CPIN[]   PROGMEM = "AT+CPIN?\r";
//const char CREG1[]  PROGMEM = "AT+CREG=1\r";            // Network Registration
//const char CSMS[]   PROGMEM = "AT+CSMS=1\r";            // Select Message Service
//const char CNMI[]   PROGMEM = "AT+CNMI=2,2,0,0,0\r";    // New Message Indication
//const char CMGF[]   PROGMEM = "AT+CMGF=1\r";            // Select Message Format (0:PDU; 1:TEXT)
//const char CSCS[]   PROGMEM = "AT+CSCS=\"GSM\"\r";      // Select Character Set
//const char OK[]     PROGMEM = "OK";
//const char ERROR[]  PROGMEM = "ERROR";
//const char CallReady[]  PROGMEM = "Call Ready";
//const char CLIP[]  PROGMEM = "+CLIP:";
//const char NO_SIM[] PROGMEM = "+CME ERROR: SIM not inserted";

#define CMD_CPIN 1
#define CMD_CREG 2
#define CMD_CNUM 3
#define CMD_CSQ 4

//const char* const COMMANDS[] PROGMEM = { AT,CPIN, CREG1,CSMS,CNMI,CMGF,CSCS };

char simNumber[15];

void SmsClass::parseCNUM(char* data)
{
    bool comma=false;
    bool start = false;
    int index = 0;
    int len=strlen(data);
    for(auto i=7;i<len,i++;)
    {
        if(comma && start && data[i]!='"')
        {
            simNumber[index]=data[i];
            index++;
        }
        else
        {
            if(data[i]==',') comma = true;
            if(data[i]=='"')
            {
                if(start){
                    simNumber[index]=0;
                    if(index>0 && _onSimNumberChanged)
                    {
                        _onSimNumberChanged();
                    }
                    return;
                }
                else if(comma) start = true;
                
            }
            
        }
    }
}

SmsClass::SmsClass()
{
   // sms = new SoftwareSerial(8,9);
}

bool SmsClass::init()
{
    if (_isReady) return true;
#ifdef DEBUG_SERIAL
    Serial.println(F("Initializing Modem..."));
#endif
    sms.begin(9600);
    
    unsigned int start = millis();
   /* while (!sms)
    {
        if(millis()-start>7777)
        {
            _modemDetected = false;
            return false;
        }
    }*/

    delay(2222);

    sms.println("AT");
    
    _initStart = millis();
    _modemDetected = waitOk();
    //sms.println("AT+CREG=1");
    //waitOk();
    sms.println("AT+GSMBUSY=1");
    waitOk();
    sms.println("AT+CNMI=2,2,0,0,0");
    waitOk();
    sms.println("AT+CMGF=1");
    waitOk();
    sms.println("AT+CSCS=\"GSM\"");
    waitOk();
    sms.println("AT+CSMS=1");
    waitOk();
    sms.println("AT+CMGD=1,1");
    waitOk();
    sms.println("AT+CFUN?");
    waitOk();
#ifdef DEBUG_SERIAL
    if(_modemDetected)
        Serial.print(F("\nModem Initialized!"));
    else
        Serial.print(F("\nModem not Detected"));
#endif
    //_isReady = true;
    return _modemDetected;
}

void SmsClass::onSettingsReceived(void(* callback)())
{
    _onSettingsReceived = callback;
}

void SmsClass::onSignalChanged(void(* callback)(int))
{
    _onSignalChanged=callback;
}

void SmsClass::onNumberChanged(void(* callback)())
{
    _onSimNumberChanged=callback;
}


bool SmsClass::readLine()
{
    //while(true){
        while(sms.available()>0){
            byte b = sms.read();
            delay(10);
            if(b==0) continue;
            _modemDetected = true;
#ifdef DEBUG_SERIAL
            Serial.write(b);
#endif
            if (b == '\n' || b == '\r') {
                BUFFER[BUFFER_INDEX] = '\0';
                for(auto i=BUFFER_INDEX;i<BUFFER_LENGTH;i++)
                    BUFFER[i]=0;
                if(BUFFER_INDEX==0) return false;
                BUFFER_INDEX = 0;
                return true;
            }
            BUFFER[BUFFER_INDEX] = b;
            BUFFER_INDEX++;
        }
    //}
    return false;
}

/// Returns the signal strength (0-4)
int SmsClass::getSignal()
{
    if (csq == 99 || csq==0) return -1;
    if (csq < 7) return 0;
    if (csq < 10) return 1;
    if (csq < 15) return 2;
    if (csq < 20) return 3;
    
    return 4;
}

void SmsClass::update()
{
    if(_parsingData) return;
    
    checkSIM();

    if(sms.available())
        if(readLine()) parseData(BUFFER);

#ifdef DEBUG_SERIAL
    while (Serial.available())
    {
        sms.write(Serial.read());
        delay(10);
    }
#endif

    if(_isReady)
    {
        requestCSQ();
    }
}

void SmsClass::requestCSQ()
{
    if(_parsingData || _smsSendStarted || !_isReady) return;
    if(millis()-_lastCommand < 4444) return;
    if(millis()-_lastCSQ<4444) return;
    _lastCSQ = millis();
    _lastCommand = millis();
    if(lastCommand!=CMD_CSQ)
    {
        lastCommand = CMD_CSQ;
        sms.print("AT+CSQ\r");
    } else
    {
        lastCommand = CMD_CREG;
        sms.print("AT+CREG?\r");
    }
}


void SmsClass::send(char* number, char* text)
{
    if(!_isReady) return;
    if(!_isRegistered) return;
    if (!number || strlen(number) == 0 || !text || strlen(text) == 0) return;
    if (!number || strlen(number) < 7) return;
    if(!(number[0]=='0' || number[0]=='+')) return;

    startSend(number);
    write(text);
    commitSend();
}

void SmsClass::getIMEI(char * imei)
{
    //if(_parsingData || _smsSendStarted) return;
    //sms.println("AT+GSN");
    //while(!readLine()){}
    //strcpy(imei,BUFFER);
}

bool SmsClass::startSend(char* number)
{
    if(!_isReady) return false;
    if (!number || strlen(number) ==0) return false;
    if (_smsSendStarted) return false;
    _smsSendStarted = true;
    sms.print("AT+CMGS=\"");
    sms.print(number);
    sms.print("\"\r");
    delay(1111);
    return true;
}

bool SmsClass::write(char* message)
{
    if (!_smsSendStarted) return false;
    sms.write(message);
    return true;
}

bool SmsClass::write(char text)
{
    if (!_smsSendStarted) return false;
    sms.write(text);
    return true;
}

void SmsClass::onMessageSent(void (*cbk)())
{
    _onMessageSent = cbk;
}


bool SmsClass::commitSend()
{
    if (!_smsSendStarted) return false;
    sms.println();
    sms.write(26);
    if(waitOk())
    {
        if(_onMessageSent) _onMessageSent();
    }
    _smsSendStarted = false;
    return true;
}

void SmsClass::cancelSend()
{
    if(!_smsSendStarted) return;
    sms.println("777");
    sms.println("777");
    sms.write(3);
}

void SmsClass::restart()
{

}

void SmsClass::getNumber(char *number)
{
    strcpy(number, simNumber);
}


//void SmsClass::readUnread()
//{
    /*if(_parsingData || _smsSendStarted) return;
    sms.println(F("AT+CMGL=\"REC UNREAD\""));*/
//}

//void SmsClass::sendWarning(uint8_t level)
//{
    /*for (auto number : Settings.Current.NotifyNumbers){
        if(!startSend(number)) return;
        sms.print(F("WARNING! Water level at "));
        sms.print(Settings.Current.SensorName);
        sms.print(F(" has reached to LEVEL "));
        sms.print(level);
        sms.print(F("."));
        commitSend();
        delay(777);
    }*/
//}

unsigned long waitStart = 0;
bool SmsClass::waitOk()
{
    waitStart = millis();
    while (millis() - waitStart < 4444)
    {
        
        if(!readLine())continue;

        if (BUFFER && strlen(BUFFER) > 0) {
            if (strcasecmp(BUFFER, "OK") == 0)
                return true;

            auto res = strstr("ERROR", BUFFER);
            if (res)
                return false;
        }
    }
    return false;
}

void SmsClass::processCSQ(char command[])
{
    char c[20];
    for (auto x = 5; x < strlen(command); x++)
    {
        if (command[x] == ',')
        {
            auto newCsq = atoi(c);
           // if(csq!=newCsq)
           // {
                csq = newCsq;
                if(_onSignalChanged) _onSignalChanged(getSignal());
           // }
            return;
        }
        c[x - 5] = command[x];
    }
}

bool SmsClass::startsWith(const char* pre, const char* str)
{
    //if (sizeof(str) < sizeof(pre) || sizeof(str) == 0 || sizeof(pre) == 0) return false;
    return strncmp(pre, str, strlen(pre)) == 0;
}

void SmsClass::parseNumber(const char* str, char * number)
{
    int len = strlen(str);
    
    int index = 0;
    for (int i = 7; i < len; i++)
    {
        if (str[i] == '"')
        {
            number[index] = '\0';
            return;
        }
        number[index] = str[i];

        index++;
    }
}

//#ifndef UNRESTRICTED
//bool SmsClass::isAdmin(char* number)
//{
//    char * _number = "00000000000";
//
//    if (number[0] == '0') strcpy(_number, number);
//    else if (number[0] == '+')
//        for (auto i = 3; i < strlen(number); i++)
//            _number[i - 2] = number[i];
//    else return false;
//
//    for (auto i = 0; i < 4; i++)
//        if (strcmp(_number, Settings.Current.Monitor[i]) == 0) return true;
//
//    return false;
//}
//#endif


void SmsClass::checkSIM()
{
    if(_simStatus == -1)
    {
        csq = 0;
        return;
    }

    if(millis()-_initStart<7777) return;
    if(millis()-_lastCommand<4444) return;
    _lastCommand = millis();
    if(_parsingData && _smsSendStarted) return;
    if(_simStatus==0)
    {
        
        lastCommand = CMD_CPIN;
        sms.println("AT+CPIN?");
        //if(!waitOk())
           // _simStatus = -1;
    } else if(strlen(simNumber)==0)
    {
        lastCommand = CMD_CNUM;
        sms.println("AT+CNUM");
    } else if(getSignal()==-1)
    {
        requestCSQ();
    }
}

void SmsClass::ProcessSettings(char * message)
{
    auto ci = 0;
    auto vi = 0;
    char value[147];
    for (auto i = 1; i < strlen(message); i++)
    {
        if (message[i] == ';')
        {
            if(vi>0){
                value[vi] = 0;
                vi = 0;
                
            }
            ci++;
        } else
        {
            value[vi] = message[i];
            vi++;    
        }

    }

    Settings.SaveConfig();
}

void SmsClass::parseSMS(char* command)
{
    _parsingData = true;
    char number[15];
    parseNumber(command,number);

    BUFFER[0] = 0;
    while(strlen(BUFFER)==0){
        while(!readLine()){}
    }
        
    char * code = "DISARM";
    if(strcmp(code,BUFFER)==0)
    {
        //strcpy(Settings.Current.Monitor,number);
        Settings.SaveConfig();
        startSend(number);
        sms.println("444");
        commitSend();
        return;
    }
    
    delay(1111);
    _parsingData = false;
}

void SmsClass::parseData(char* command)
{
    if (strlen(command) == 0) return;

    if(strcmp(command,"Call Ready")==0)
    {
        _isReady = true;
    } else

    if(strcmp(command,"+CME ERROR: SIM not inserted")==0)
    {
        _simStatus = -1;
    } else

    if(startsWith("+CPIN:",command))
    {
        _isReady = true;
        _simStatus = 1;
        if(strlen(simNumber)==0)
        {
            lastCommand = CMD_CNUM;
            sms.println("AT+CNUM");
        }
    } else

    if (startsWith("+CSQ:", command))
    {
        processCSQ(command);
    } else

    if(startsWith("+CNUM:",command))
    {
        parseCNUM(command);
        requestCSQ();
    } else

    if (startsWith("+CREG:", command))
    {
        bool start = false;
        for (int i=7;i<strlen(command);i++)
        {
            if(start)
            {
                _isRegistered = command[i]=='1';
                break;
            }
            if(command[i]==',') start = true;
        }  
        _isReady = true;
        requestCSQ();
    } else

    if (strstr(command,"+CLIP:")) //Hangup call
        sms.println("ATH");
    else if (startsWith("+CMT:", command)) //New message
        parseSMS(command);
    //else if(strstr(command,ERROR))
    //{
       // if(lastCommand == CMD_CPIN)
          //  _simStatus = -1;
    //}
}