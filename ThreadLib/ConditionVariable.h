#ifndef __CONDITION_VARIABLE_H__
#define __CONDITION_VARIABLE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "Mutex.h"


//Large enough to hold a Windows CONDITION_VARIABLE or a Linux pthread_cond_t.
#define CONDITION_VARIABLE_HANDLE_SIZE		64


class CConditionVariable
{
protected:
	alignas(8) uint8	maCondition[CONDITION_VARIABLE_HANDLE_SIZE];

public:
	void	Init(void);
	void	Kill(void);

	//pcMutex must be locked.  It is unlocked while waiting and locked again
	//before Wait returns.  Wait can return without a wake so check the
	//condition again.
	void	Wait(CMutex* pcMutex);
	void	WakeOne(void);
	void	WakeAll(void);
};


#endif // __CONDITION_VARIABLE_H__

