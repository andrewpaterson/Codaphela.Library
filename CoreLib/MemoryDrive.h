#ifndef __MEMORY_DRIVE_H__
#define __MEMORY_DRIVE_H__
#include "FileDrive.h"


class CMemoryDrive : public CFileDrive
{
protected:
	void*	mpvData;

public:
	bool	Init(size_t uiSizeInBytes, uint32 uiSectorSize);
	void	Kill(void);

	bool	Read(uint64 uiSector, void* pvData);
	bool	Write(uint64 uiSector, void* pvData);
	bool    Erase(uint64 uiStartSector, uint64 uiStopSectorInclusive);

	void*	GetMemory(void);
};


#endif // __MEMORY_DRIVE_H__
