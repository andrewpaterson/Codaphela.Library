/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "DatasIO.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDatasIO::Init(void)
{
	mcDataIOs.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDatasIO::Kill(void)
{
	mcDataIOs.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
DataIO_FileWriter CDatasIO::GetFileWriter(char* szClassName)
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
DataIO_FileReader CDatasIO::GetFileReader(char* szClassName)
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
SDataIO* CDatasIO::GetIO(char* szClassName)
{
	return (SDataIO*)mcDataIOs.Get(szClassName, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SDataIO* CDatasIO::GetIO(const char* szClassName)
{
	return (SDataIO*)mcDataIOs.Get(szClassName, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDatasIO::NumDataIOs(void)
{
	return mcDataIOs.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SDataTypeIO::Save(CFileWriter* pcFile)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SDataTypeIO::Load(CFileReader* pcFile)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDatasIO::Add(const char* szFilename, DataIO_FileWriter fSpecificClassFileSave, DataIO_FileReader fSpecificClassFileLoad)
{
	SDataIO*	psIO;

	psIO = (SDataIO*)mcDataIOs.Put(szFilename, sizeof(SDataIO));
	psIO->fWriter = (DataIO_FileWriter)fSpecificClassFileSave;
	psIO->fReader = (DataIO_FileReader)fSpecificClassFileLoad;
}

