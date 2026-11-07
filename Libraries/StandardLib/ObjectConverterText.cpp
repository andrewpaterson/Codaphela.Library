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
#include "BaseLib/TextFile.h"
#include "Unknowns.h"
#include "Objects.h"
#include "String.h"
#include "ObjectSourceText.h"
#include "ObjectConverterText.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectConverterText::Init(CObjects* pcObjects)
{
	CObjectConverter::Init(pcObjects);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectConverterText::Kill(void)
{
	CObjectConverter::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CObjectConverterText::GetFileExtension(void)
{
	return "TXT";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectConverterText::IsFor(CAbstractFile* pcFile)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CObjectSource* CObjectConverterText::CreateSource(CAbstractFile* pcFile, const char* szFilename)
{
	CObjectSourceText*	pcSource;

	if ((!pcFile) || (!szFilename))
	{
		return NULL;
	}

	pcSource = UMalloc(CObjectSourceText);
	pcSource->Init(this, pcFile, szFilename);
	return pcSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectConverterText::Convert(CObjectSource* pcSource, const char* szObjectName)
{
	CTextFile			cTextFile;
	Ptr<CString>		pString;
	bool				bResult;

	cTextFile.Init();

	bResult = cTextFile.Read(pcSource->GetFile());
	if (!bResult)
	{
		cTextFile.Kill();
		return NULL;
	}

	//Converted objects are named: it is how objects in the Codabase reference them.
	pString = mpcObjects->Malloc<CString>(szObjectName);
	if (pString.IsNull())
	{
		cTextFile.Kill();
		return NULL;
	}
	pString->Init(cTextFile.Text());
	cTextFile.Kill();
	return (CBaseObject*)pString.Return();
}

