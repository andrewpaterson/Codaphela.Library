#ifndef __MUTEX_H__
#define __MUTEX_H__
#include "PrimitiveTypes.h"


//Large enough to hold a Windows SRWLOCK or a Linux pthread_mutex_t.
#define MUTEX_HANDLE_SIZE		48


//A mutex between the threads of one process.  It is not recursive.
class CMutex
{
friend class CConditionVariable;
protected:
	alignas(8) uint8	maMutex[MUTEX_HANDLE_SIZE];

public:
	void	Init(void);
	void	Kill(void);

	void	Lock(void);
	void	Unlock(void);
};


#endif // __MUTEX_H__

