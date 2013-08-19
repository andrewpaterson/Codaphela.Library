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
#include "ObjectConverter.h"
#include "Objects.h"
#include "ObjectSingleSource.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectSingleSource::Init(CObjectConverter* pcConverter, CAbstractFile* pcFile, char* szObjectName)
{
	CObjectSource::Init(pcConverter, pcFile, NULL);
	mszObjectName.Init(szObjectName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectSingleSource::Kill(void)
{
	mszObjectName.Kill();
	CObjectSource::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectSingleSource::Contains(char* szFullName)
{
	return mszObjectName.Equals(szFullName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectSingleSource::Convert(char* szFullName)
{
	if (mszObjectName.EqualsIgnoreCase(szFullName))
	{
		return mpcConverter->Convert(this, szFullName);
	}
	else
	{
		return NULL;
	}
}

