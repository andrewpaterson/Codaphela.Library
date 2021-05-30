/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "Logger.h"
#include "DataTypesIO.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDataTypesIO::Init(void)
{
	mcDataIOs.Init(32);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDataTypesIO::Kill(void)
{
	mcDataIOs.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
DataTypeIO_FileWriter CDataTypesIO::Save(char* szClassName)
{
	SDataIO* psIO;

	psIO = (SDataIO*)mcDataIOs.Get(szClassName);
	if (psIO)
	{
		return psIO->fWriter;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
DataTypeIO_FileReader CDataTypesIO::Load(char* szClassName)
{
	SDataIO* psIO;

	psIO = (SDataIO*)mcDataIOs.Get(szClassName);
	if (psIO)
	{
		return psIO->fReader;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
 SDataIO* CDataTypesIO::GetIO(char* szClassName)
{
	return (SDataIO*)mcDataIOs.Get(szClassName, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CDataTypesIO::NumDataIOs(void)
{
	return mcDataIOs.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL SDataTypeIO::Save(CFileWriter* pcFile)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL SDataTypeIO::Load(CFileReader* pcFile)
{
	return TRUE;
}


