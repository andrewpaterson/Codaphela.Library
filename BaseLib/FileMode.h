/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __FILE_MODE_H__
#define __FILE_MODE_H__
#include "Define.h"


//---------------------------------
// EFM = Enum File Mode
//---------------------------------
enum EFileMode
{
	EFM_Unknown,
	EFM_Write_Create,
	EFM_Read,
	EFM_Read_Create,
	EFM_ReadWrite,
	EFM_ReadWrite_Create,
};


//---------------------------------
// EFSO = Enum File Seek Origin
//---------------------------------
enum EFileSeekOrigin
{
	EFSO_SET     = 0,
	EFSO_CURRENT = 1,
	EFSO_END     = 2  //Seeking to the END of file is not standard supported.  Try not to use it.
};


BOOL IsFileModeWritable(EFileMode eFileMode);
BOOL IsFileModeReadable(EFileMode eFileMode);
BOOL IsFileModeReadOnly(EFileMode eFileMode);
BOOL IsFileModeCreate(EFileMode eFileMode);


#endif // !__FILE_MODE_H__

