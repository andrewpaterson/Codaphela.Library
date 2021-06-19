#ifndef __INTER_PROCESS_FLOW_H__
#define __INTER_PROCESS_FLOW_H__
#include "SharedMemory.h"


class CInterProcessFlow
{
protected:
	CSharedMemory		mcSharedMemory;

public:
	void Init(char* szSharedMemoryName);
	void Init(char* szSharedMemoryName, char* szSharedMemoryNamePostfix);
	void Kill(void);
};


#endif // !__INTER_PROCESS_FLOW_H__


