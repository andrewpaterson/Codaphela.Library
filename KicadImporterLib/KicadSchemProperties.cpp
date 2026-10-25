/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela KicadImporterLib

Codaphela KicadImporterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela KicadImporterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela KicadImporterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/StringHelper.h"
#include "KicadSchemProperties.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemProperties::Init(void)
{
	macProperties.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemProperties::Kill(void)
{
	size	i;
	size	uiNumElements;

	uiNumElements = macProperties.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		GetProperty(i)->Kill();
	}
	macProperties.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CKicadSchemProperty* CKicadSchemProperties::Add(char* szName, char* szValue)
{
	CKicadSchemProperty*	pcProperty;

	pcProperty = (CKicadSchemProperty*)macProperties.Add(sizeof(CKicadSchemProperty));
	pcProperty->Init(szName, szValue);
	return pcProperty;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CKicadSchemProperty* CKicadSchemProperties::Get(const char* szName)
{
	size					i;
	size					uiNumElements;
	CKicadSchemProperty*	pcProperty;

	uiNumElements = macProperties.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcProperty = GetProperty(i);
		if (StringCompare(pcProperty->GetName(), szName) == 0)
		{
			return pcProperty;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CKicadSchemProperties::GetValue(const char* szName)
{
	CKicadSchemProperty*	pcProperty;

	pcProperty = Get(szName);
	if (pcProperty == NULL)
	{
		return NULL;
	}
	return pcProperty->GetValue();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CKicadSchemProperties::NumProperties(void)
{
	return macProperties.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CKicadSchemProperty* CKicadSchemProperties::GetProperty(size uiIndex)
{
	return (CKicadSchemProperty*)macProperties.Get(uiIndex);
}

