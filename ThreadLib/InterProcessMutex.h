#ifndef __INTER_PROCESS_MUTEX_H__
#define __INTER_PROCESS_MUTEX_H__
#include "BaseLib/Chars.h"


//Large enough to hold a Windows mutex HANDLE or a Linux shared mutex file descriptor and pointer.
#define INTER_PROCESS_MUTEX_HANDLE_SIZE		8


class CInterProcessMutex
{
protected:
	CChars	mszName;
	alignas(8) uint8	maMutex[INTER_PROCESS_MUTEX_HANDLE_SIZE];

public:
	void Init(char* szName);
	void Kill(void);

	bool Create(void);
	bool Connect(void);
	void Close(void);
	bool Lock(void);
	bool Unlock(void);
};


#endif // __INTER_PROCESS_MUTEX_H__

