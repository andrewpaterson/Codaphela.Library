#ifndef __INTER_PROCESS_HOLD_H__
#define __INTER_PROCESS_HOLD_H__
#include "InterProcessCounter.h"


//Holds other processes until it is Stopped.  The holding process Starts it,
//the held processes Touch it and Wait.
class CInterProcessHold : public CInterProcessCounter
{
public:
	void Start(void);
	bool Touch(void);
	bool Wait(int iTimeoutMilliseconds = ATOMIC_WAIT_FOREVER);
	void Stop(void);
};


#endif // __INTER_PROCESS_HOLD_H__

