/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "ArrayBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlock::WriteArrayUnknown(CFileWriter* pcFileWriter)
{
	if (!pcFileWriter->WriteData(this, sizeof(CArrayBlock))) 
	{ 
		return FALSE; 
	}

	if (NumElements() != 0)
	{
		if (!pcFileWriter->WriteData(GetData(), ByteSize())) 
		{ 
			return FALSE; 
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlock::ReadArrayUnknown(CFileReader* pcFileReader)
{
	if (!pcFileReader->ReadData(this, sizeof(CArrayBlock))) 
	{ 
		return FALSE; 
	}

	if (NumElements() != 0)
	{
		InitFromHeader();
		if (!pcFileReader->ReadData(GetData(), ByteSize())) 
		{ 
			return FALSE; 
		}
	}
	return TRUE;
}

