#ifndef __LOG_TO_MEMORY_H__
#define __LOG_TO_MEMORY_H__
#include "MemoryFile.h"
#include "LogConfig.h"


class CLogToMemory
{
private:
	CMemoryFile	mcMemoryFile;
	SLogConfig	msSilentConfig;
	bool		mbSetSilent;

public:
	void Start(bool bSetSilent);
	void Stop(char* szOutput, size_t uiOutputSize);
};


#endif // __LOG_TO_MEMORY_H__

