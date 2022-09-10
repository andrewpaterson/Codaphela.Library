#include "FileMode.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsFileModeWritable(EFileMode eFileMode)
{
	if ((eFileMode == EFM_ReadWrite) || (eFileMode == EFM_Write_Create) || (eFileMode == EFM_ReadWrite_Create))
	{
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsFileModeReadable(EFileMode eFileMode)
{
	if ((eFileMode == EFM_ReadWrite) || (eFileMode == EFM_Read) || (eFileMode == EFM_ReadWrite_Create) || (eFileMode == EFM_Read_Create))
	{
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsFileModeReadOnly(EFileMode eFileMode)
{
	if ((eFileMode == EFM_Read) || (eFileMode == EFM_Read_Create))
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool IsFileModeCreate(EFileMode eFileMode)
{
	if ((eFileMode == EFM_Read_Create) || (eFileMode == EFM_Write_Create) || (eFileMode == EFM_ReadWrite_Create))
	{
		return true;
	}
	else
	{
		return false;
	}
}

