#ifndef __MEMORY_DRIVE_H__
#define __MEMORY_DRIVE_H__
#include "FileDrive.h"


class CMemoryDrive : public CFileDrive
{
protected:
	SDriveSector*	mapsSectors;

public:
	bool	Init(size_t uiSizeInBytes);
	void	Kill(void);

	void	Read(uint64 uiSector, SDriveSector* psSector);
	void	Write(uint64 uiSector, SDriveSector* psSector);
};


#endif // __MEMORY_DRIVE_H__

