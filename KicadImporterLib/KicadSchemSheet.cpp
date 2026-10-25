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
#include "KicadSchemSheet.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemSheet::Init(SKicadSchemPoint* psPosition, SKicadSchemPoint* psSize)
{
	msPosition = *psPosition;
	msSize = *psSize;
	mszUuid.Init();
	mcProperties.Init();
	macPins.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemSheet::Kill(void)
{
	size	i;
	size	uiNumElements;

	uiNumElements = macPins.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		GetPin(i)->Kill();
	}
	macPins.Kill();
	mcProperties.Kill();
	mszUuid.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemSheet::SetUuid(char* szUuid)
{
	mszUuid.Set(szUuid);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CKicadSchemSheetPin* CKicadSchemSheet::AddPin(char* szName, EKicadSchemLabelShape eShape)
{
	CKicadSchemSheetPin*	pcPin;

	pcPin = (CKicadSchemSheetPin*)macPins.Add(sizeof(CKicadSchemSheetPin));
	pcPin->Init(szName, eShape);
	return pcPin;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SKicadSchemPoint* CKicadSchemSheet::GetPosition(void)
{
	return &msPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SKicadSchemPoint* CKicadSchemSheet::GetSize(void)
{
	return &msSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CKicadSchemSheet::GetUuid(void)
{
	return mszUuid.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CKicadSchemProperties* CKicadSchemSheet::GetProperties(void)
{
	return &mcProperties;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CKicadSchemSheet::GetSheetName(void)
{
	char*	szName;

	//KiCad 6 named the fields "Sheet name" and "Sheet file".
	szName = mcProperties.GetValue("Sheetname");
	if (szName == NULL)
	{
		szName = mcProperties.GetValue("Sheet name");
	}
	return szName;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CKicadSchemSheet::GetSheetFile(void)
{
	char*	szFile;

	szFile = mcProperties.GetValue("Sheetfile");
	if (szFile == NULL)
	{
		szFile = mcProperties.GetValue("Sheet file");
	}
	return szFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CKicadSchemSheet::NumPins(void)
{
	return macPins.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CKicadSchemSheetPin* CKicadSchemSheet::GetPin(size uiIndex)
{
	return (CKicadSchemSheetPin*)macPins.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CKicadSchemSheetPin* CKicadSchemSheet::GetPin(char* szName)
{
	size					i;
	size					uiNumElements;
	CKicadSchemSheetPin*	pcPin;

	uiNumElements = macPins.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcPin = GetPin(i);
		if (StringCompare(pcPin->GetName(), szName) == 0)
		{
			return pcPin;
		}
	}
	return NULL;
}

