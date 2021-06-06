#include "Logger.h"
#include "DataTypesIO.h"
#include "Int2.h"
#include "Int3.h"
#include "Int4.h"
#include "GeometricTypes.h"
#include "TypeNames.h"
#include "GlobalDataTypesIO.h"


CDatasIO	gcDataTypesIO;
BOOL		gbDataTypesIO = FALSE;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DataIOInit(void)
{
	if (gbDataTypesIO)
	{
		gcLogger.Error("Global Data Types IO has already been initialised.");
	}

	TypesValidate();

	gcDataTypesIO.Init();
	gbDataTypesIO = TRUE;

	gcDataTypesIO.Add<SIOInt8>(gcTypeNames.GetPrettyName(PT_int8));
	gcDataTypesIO.Add<SIOInt16>(gcTypeNames.GetPrettyName(PT_int16));
	gcDataTypesIO.Add<SIOInt32>(gcTypeNames.GetPrettyName(PT_int32));
	gcDataTypesIO.Add<SIOInt64>(gcTypeNames.GetPrettyName(PT_int64));
	gcDataTypesIO.Add<SIOUInt8>(gcTypeNames.GetPrettyName(PT_uint8));
	gcDataTypesIO.Add<SIOUInt16>(gcTypeNames.GetPrettyName(PT_uint16));
	gcDataTypesIO.Add<SIOUInt32>(gcTypeNames.GetPrettyName(PT_uint32));
	gcDataTypesIO.Add<SIOUInt64>(gcTypeNames.GetPrettyName(PT_uint64));

	gcDataTypesIO.Add<SIOBool>(gcTypeNames.GetPrettyName(PT_bool));

	gcDataTypesIO.Add<SIOChar8>(gcTypeNames.GetPrettyName(PT_char8));
	gcDataTypesIO.Add<SIOChar16>(gcTypeNames.GetPrettyName(PT_char16));

	gcDataTypesIO.Add<SIOFloat32>(gcTypeNames.GetPrettyName(PT_float32));
	gcDataTypesIO.Add<SIOFloat64>(gcTypeNames.GetPrettyName(PT_float64));

	gcDataTypesIO.Add<SInt2>(gcTypeNames.GetPrettyName(PT_M2int32));
	gcDataTypesIO.Add<SInt3>(gcTypeNames.GetPrettyName(PT_M3int32));
	gcDataTypesIO.Add<SInt4>(gcTypeNames.GetPrettyName(PT_M4int32));

	gcDataTypesIO.Add<SFloat2>(gcTypeNames.GetPrettyName(PT_M2float32));
	gcDataTypesIO.Add<SFloat3>(gcTypeNames.GetPrettyName(PT_M3float32));
	gcDataTypesIO.Add<SFloat4>(gcTypeNames.GetPrettyName(PT_M4float32));
	gcDataTypesIO.Add<SFloat4x4>(gcTypeNames.GetPrettyName(PT_M4x4float32));
	gcDataTypesIO.Add<SDouble2>(gcTypeNames.GetPrettyName(PT_M2float64));
	gcDataTypesIO.Add<SDouble3>(gcTypeNames.GetPrettyName(PT_M3float64));
	gcDataTypesIO.Add<SDouble4>(gcTypeNames.GetPrettyName(PT_M4float64));
	gcDataTypesIO.Add<SDouble4x4>(gcTypeNames.GetPrettyName(PT_M4x4float64));

	gcDataTypesIO.Add<SIOVoid>(gcTypeNames.GetPrettyName(PT_void));
	gcDataTypesIO.Add<SIOData>(gcTypeNames.GetPrettyName(PT_Data));

	gcDataTypesIO.Add("CChars", (DataIO_FileWriter)(&CChars::WriteString), (DataIO_FileReader)(&CChars::ReadString));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DataIOKill(void)
{
	if (!gbDataTypesIO)
	{
		gcLogger.Error("Global Data Types IO has already been Killed.");
	}

	gcDataTypesIO.Kill();
	gbDataTypesIO = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL DataIOValidate(void)
{
	if (!gbDataTypesIO)
	{
		gcLogger.Error("Global Data Types IO has not been initialised.  DataIOInit.");
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


