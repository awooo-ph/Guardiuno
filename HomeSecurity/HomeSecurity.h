// HomeSecurity.h

#ifndef _HOMESECURITY_h
#define _HOMESECURITY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Sms.h"
#include "Motion.h"
#include "Settings.h"
#include "RemoteControl.h"

class HomeSecurityClass
{
 protected:


 public:
	void init();
};

extern HomeSecurityClass HomeSecurity;

#endif

