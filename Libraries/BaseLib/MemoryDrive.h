#ifndef __MEMORY_DRIVE_H__
#define __MEMORY_DRIVE_H__
#include "FileDrive.h"


class CMemoryDrive : public CFileDrive
{
protected:
	void*	mpvData;
	uint64	muiDataSize;

public:
	bool	Init(uint64 uiSizeInBytes, uint32 uiSectorSize);
	void	Kill(void) override;

	bool	Read(uint64 uiSector, void* pvData) override;
	bool	Read(uint64 uiSector, uint32 uiNumSectors, void* pvData) override;
	bool	Write(uint64 uiSector, void* pvData) override;
	bool	Write(uint64 uiSector, uint32 uiNumSectors, void* pvData) override;
	bool	Erase(uint64 uiStartSector, uint64 uiStopSectorInclusive) override;

	bool    Erase(void);

	void*	GetMemory(void) const;
};


#endif // __MEMORY_DRIVE_H__

