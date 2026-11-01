#ifndef __TEST_FAT_32_COMMON_H__
#define __TEST_FAT_32_COMMON_H__
#include <stdlib.h>
#include <string.h>
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/FileDrive.h"
#include "BaseLib/DiskFile.h"


#define FAT_TEST_PAGE_SECTORS		64
#define FAT_TEST_PAGE_BYTES			((uint32)FAT_TEST_PAGE_SECTORS * 512)


char* AllocateStringBuffer(lsize uiSize);


//A drive that keeps only the pages of sectors used, so it can be as big as FAT32 needs (65525 clusters) in a little
//memory.  The pages not written read as zeros, or (InitImage) as a disk image file's sectors, read when first used:
//a 64MB image doesn't fit in the W65C816's 16MB.  Its pages are smaller than 64KB (a W65C816 size).
class CFatTestDrive : public CFileDrive
{
protected:
	uint8**		mapuiPages;
	uint32		muiPages;
	CDiskFile	mcImage;
	bool		mbImage;
	uint64		muiImageSectors;
	uint32		muiWrites;

public:
	void	Init(uint64 uiSectors);
	void	Init(uint64 uiBytes, uint32 uiSectorSize);
	bool	InitImage(const char* szFilename);
	void	Kill(void);

	bool	Read(uint64 uiSector, void* pvData);
	bool	Read(uint64 uiSector, uint32 uiNumSectors, void* pvData);
	bool	Write(uint64 uiSector, void* pvData);
	bool	Write(uint64 uiSector, uint32 uiNumSectors, void* pvData);
	bool	Erase(uint64 uiStartSector, uint64 uiStopSectorInclusive);
	bool	Erase(void);

	uint32	NumWrites(void);

protected:
	uint8*	GetPage(uint32 uiPage, bool bWrite);
};


#endif // __TEST_FAT_32_COMMON_H__

