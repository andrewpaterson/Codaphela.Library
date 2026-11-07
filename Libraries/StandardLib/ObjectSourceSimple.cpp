/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "Unknowns.h"
#include "ObjectSourceSimple.h"
#include "FileObjectReader.h"
#include "ObjectConverterNative.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectSourceSimple::Init(CObjectConverter* pcConverter, CAbstractFile* pcFile, const char* szFilename)
{
	CObjectSingleSource::Init(pcConverter, pcFile, szFilename);
	mcFile.Init(pcFile);
	mpcReader = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectSourceSimple::Kill(void)
{
	mcFile.Kill();
	CObjectSingleSource::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectSourceSimple::IsNative(void)
{
	return true; 
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectSourceSimple::Convert(const char* szFullName)
{
	CBaseObject*	pcObject;

	if (!Contains(szFullName))
	{
		return NULL;
	}

	mpcReader = UMalloc(CFileObjectReader);
	mpcReader->Init(&mcFile);

	pcObject = mpcConverter->Convert(this, szFullName);
	mpcReader->Kill();
	mpcReader = NULL;

	return pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CExternalObjectReader* CObjectSourceSimple::GetReader(void)
{
	return mpcReader;
}
