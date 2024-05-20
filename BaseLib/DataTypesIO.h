#ifndef __DATA_TYPES_IO_H__
#define __DATA_TYPES_IO_H__
#include "Constructable.h"
#include "PrimitiveTypes.h"
#include "DataIO.h"


#define DATA_TYPE_IO_DEFINITION(CLASS_TYPE, C_PRIMITIVE, WRITE_FUNC, READ_FUNC) \
struct CLASS_TYPE \
{ \
CONSTRUCTABLE(CLASS_TYPE); \
 \
	C_PRIMITIVE	mVal; \
 \
	bool Save(CFileWriter* pcFile) \
	{ \
		return pcFile-> WRITE_FUNC (mVal); \
	} \
 \
	bool Load(CFileReader* pcFile) \
	{ \
		return pcFile-> READ_FUNC (&mVal); \
	} \
};


DATA_TYPE_IO_DEFINITION(SIOInt8, int8, WriteByte, ReadByte)
DATA_TYPE_IO_DEFINITION(SIOInt16, int16, WriteShort, ReadShort)
DATA_TYPE_IO_DEFINITION(SIOInt32, int32, WriteInt32, ReadInt32)
DATA_TYPE_IO_DEFINITION(SIOInt64, int64, WriteLong, ReadLong)
DATA_TYPE_IO_DEFINITION(SIOUInt8, uint8, WriteByte, ReadByte)
DATA_TYPE_IO_DEFINITION(SIOUInt16, uint16, WriteShort, ReadShort)
DATA_TYPE_IO_DEFINITION(SIOUInt32, uint32, WriteInt32, ReadInt32)
DATA_TYPE_IO_DEFINITION(SIOUInt64, uint64, WriteLong, ReadLong)

DATA_TYPE_IO_DEFINITION(SIOBool, bool, WriteBool, ReadBool)

DATA_TYPE_IO_DEFINITION(SIOChar8, char8, WriteChar, ReadChar)
DATA_TYPE_IO_DEFINITION(SIOChar16, char16, WriteWChar, ReadWChar)

DATA_TYPE_IO_DEFINITION(SIOFloat32, float32, WriteFloat, ReadFloat)
DATA_TYPE_IO_DEFINITION(SIOFloat64, float64, WriteFloat, ReadFloat)


struct SIOVoid
{
CONSTRUCTABLE(SIOVoid);
	bool Save(CFileWriter* pcFile)
	{
		return true;
	}

	bool Load(CFileReader* pcFile)
	{
		return true;
	}
};


struct SIOData
{
CONSTRUCTABLE(SIOData);
	bool Save(CFileWriter* pcFile)
	{
		return pcFile->WriteData(this, 1); 
	}

	bool Load(CFileReader* pcFile)
	{
		return pcFile->ReadData(this, 1);
	}
};


#endif // __DATA_TYPES_IO_H__

