#ifndef __FILE_DRIVE_H__
#define __FILE_DRIVE_H__
#include "BaseLib/PrimitiveTypes.h"


#define STORAGE_SUCCESS							0x0
#define STORAGE_VOLTAGE_NOT_SUPPORTED			0x0001
#define STORAGE_INVALID_MEDIA					0x0002
#define STORAGE_UNKNOWN_ERROR					0x0003
#define STORAGE_INVALID_PARAMETER				0x0004
#define STORAGE_ADDRESS_ERROR					0x0005
#define STORAGE_ERASE_SEQ_ERROR					0x0006
#define STORAGE_CRC_ERROR						0x0007
#define STORAGE_ILLEGAL_COMMAND					0x0008
#define STORAGE_ERASE_RESET						0x0009
#define STORAGE_COMMUNICATION_ERROR				0x000A
#define STORAGE_TIMEOUT							0x000B
#define STORAGE_IDLE							0x000C
#define STORAGE_OUT_OF_RANGE					0x000D
#define STORAGE_MEDIA_ECC_FAILED				0x000E
#define STORAGE_CC_ERROR						0x000F
#define STORAGE_OP_IN_PROGRESS					0x0010
#define STORAGE_MEDIUM_WRITE_PROTECTED			0x0011
#define STORAGE_OUT_OF_SPACE					0x0012
#define STORAGE_DEVICE_NOT_READY				0x0013
#define STORAGE_AWAITING_DATA					0x0014
#define STORAGE_INVALID_MULTI_BLOCK_RESPONSE	0x0015

#define STORAGE_MULTI_SECTOR_RESPONSE_STOP		0x0
#define STORAGE_MULTI_SECTOR_RESPONSE_SKIP		0x1
#define STORAGE_MULTI_SECTOR_RESPONSE_READY		0x2


class CFileDrive
{
protected:
	uint64	muiMaxSector;  //Not inclusive
	uint32	muiSectorSize;

public:
			void	Init(uint32	uiSectorSize);
	virtual void	Kill(void) =0;

	virtual bool	Read(uint64 uiSector, void* pvData) =0;
	virtual bool	Write(uint64 uiSector, void* pvData) =0;
	virtual bool    Erase(uint64 uiStartSector, uint64 uiStopSectorInclusive) =0;
			uint64	GetTotalSectors(void);
			uint32	GetSectorSize(void);
	virtual uint32	GetPageSize(void);
};


#endif // !__FILE_DRIVE_H__

