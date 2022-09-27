#ifndef __FILE_DRIVE_H__
#define __FILE_DRIVE_H__
#include "BaseLib/PrimitiveTypes.h"


class CFileDrive
{
protected:
	uint64	muiMaxSector;  //Not inclusive
	uint32	muiSectorSize;

public:
			void	Init(uint32	uiSectorSize);
	virtual void	Kill(void) =0;

	virtual bool	Read(uint64 uiSector, void* pvData) =0;
	virtual bool	Read(uint64 uiSector, uint32 uiNumSectors, void* pvData) =0;
	virtual bool	Write(uint64 uiSector, void* pvData) =0;
	virtual bool	Write(uint64 uiSector, uint32 uiNumSectors, void* pvData) =0;
	virtual bool    Erase(uint64 uiStartSector, uint64 uiStopSectorInclusive) =0;
			uint64	GetTotalSectors(void);
			uint32	GetSectorSize(void);
	virtual uint32	GetPageSize(void);

			uint64	SetMaxSectorForTesting(uint64 uiMaxSector);
};


#endif // !__FILE_DRIVE_H__

