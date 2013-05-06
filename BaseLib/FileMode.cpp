#include "FileMode.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL IsFileModeWritable(EFileMode eFileMode)
{
	if ((eFileMode == EFM_ReadWrite) || (eFileMode == EFM_Write_Create) || (eFileMode == EFM_ReadWrite_Create))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL IsFileModeReadable(EFileMode eFileMode)
{
	if ((eFileMode == EFM_ReadWrite) || (eFileMode == EFM_Read) || (eFileMode == EFM_ReadWrite_Create) || (eFileMode == EFM_Read_Create))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL IsFileModeReadOnly(EFileMode eFileMode)
{
	if ((eFileMode == EFM_Read) || (eFileMode == EFM_Read_Create))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


