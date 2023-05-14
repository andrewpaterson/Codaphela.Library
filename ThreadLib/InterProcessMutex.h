#ifndef __INTER_PROCESS_MUTEX_H__
#define __INTER_PROCESS_MUTEX_H__
#include "BaseLib/WindowsHeaders.h"
#include "BaseLib/Chars.h"


class CInterProcessMutex
{
protected:
	CChars	mszName;
	HANDLE	mhMutex;

public:
	void Init(char* szName);
	void Kill(void);

	bool Create(void);
	bool Connect(void);
	void Close(void);
	bool Lock(void);
	bool Unlock(void);
};


#endif // !__INTER_PROCESS_MUTEX_H__

