#ifndef __FILE_DRIVE_H__
#define __FILE_DRIVE_H__
#include "BaseLib/PrimitiveTypes.h"


#define DRIVE_SECTOR_SIZE	512


struct SDriveSector
{
	uint8	uiSector[DRIVE_SECTOR_SIZE];
};


class CFileDrive
{
protected:
	uint64 muiMaxSector;  //Not inclusive

public:
			void	Init(void);
	virtual void	Kill(void) =0;

	virtual bool	Read(uint64 uiSector, SDriveSector* psSector) =0;
	virtual bool	Write(uint64 uiSector, SDriveSector* psSector) =0;
			bool	Read(uint64 uiSector, void* pvData);
			bool	Write(uint64 uiSector, void* pvData);
};


#endif // !__FILE_DRIVE_H__

