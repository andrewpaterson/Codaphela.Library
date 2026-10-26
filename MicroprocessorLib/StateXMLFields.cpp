/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela MicroprocessorLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MicroprocessorLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MicroprocessorLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdint.h>
#include <string.h>
#include "BaseLib/Logger.h"
#include "BaseLib/StringHelper.h"
#include "StateXMLFields.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool LoadStateBool(CMarkupTag* pcTag, const char* szName, bool* pbValue)
{
	if (!pcTag->GetBoolAttribute(szName, pbValue))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] ", szName, " must be true or false.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool LoadStateInt64(CMarkupTag* pcTag, const char* szName, int64 iMinimum, int64 iMaximum, int64* piValue)
{
	if (!pcTag->GetInt64Attribute(szName, piValue) || (*piValue < iMinimum) || (*piValue > iMaximum))
	{
		*piValue = 0;
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] ", szName, " must be a whole number in range.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The index of szValue in aszNames, or -1 (and an error logged).
//
//////////////////////////////////////////////////////////////////////////
int LoadStateName(CMarkupTag* pcTag, const char* szName, const char** aszNames, int iNumNames)
{
	char*	szValue;
	int		i;

	szValue = pcTag->GetStringAttribute(szName);
	if (szValue != NULL)
	{
		for (i = 0; i < iNumNames; i++)
		{
			if (StringCompare(szValue, aszNames[i]) == 0)
			{
				return i;
			}
		}
	}
	gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] has no ", szName, " or an unknown one [", (szValue != NULL) ? szValue : "", "].", NULL);
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The field's value, sign extended if it's signed.
//
//////////////////////////////////////////////////////////////////////////
static int64 GetStateXMLField(const SStateXMLField* psField, void* pvState)
{
	uint64	uiValue;
	uint64	uiSign;

	uiValue = 0;
	memcpy(&uiValue, (uint8*)pvState + psField->uiOffset, psField->uiBytes);
	if ((psField->eType == SXFT_Signed) && (psField->uiBytes < 8))
	{
		uiSign = (uint64)1 << (psField->uiBytes * 8 - 1);
		uiValue = (uiValue ^ uiSign) - uiSign;
	}
	return (int64)uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
// Every field as an attribute.
//
//////////////////////////////////////////////////////////////////////////
void SaveStateXMLFields(CXMLWriter* pcWriter, const SStateXMLField* asFields, size uiNumFields, void* pvState)
{
	size	i;

	for (i = 0; i < uiNumFields; i++)
	{
		if (asFields[i].eType == SXFT_Bool)
		{
			pcWriter->Attribute(asFields[i].szName, *((bool*)((uint8*)pvState + asFields[i].uiOffset)));
		}
		else
		{
			pcWriter->Attribute(asFields[i].szName, GetStateXMLField(&asFields[i], pvState));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Every field from its attribute, each in its type's range.  Every field
// is tried, so each missing or bad one is logged; false if any was.
//
//////////////////////////////////////////////////////////////////////////
bool LoadStateXMLFields(CMarkupTag* pcTag, const SStateXMLField* asFields, size uiNumFields, void* pvState)
{
	const SStateXMLField*	psField;
	size					i;
	bool					bResult;
	int64					iValue;
	int64					iMinimum;
	int64					iMaximum;
	uint32					uiBits;

	bResult = true;
	for (i = 0; i < uiNumFields; i++)
	{
		psField = &asFields[i];
		if (psField->eType == SXFT_Bool)
		{
			bResult &= LoadStateBool(pcTag, psField->szName, (bool*)((uint8*)pvState + psField->uiOffset));
			continue;
		}

		uiBits = psField->uiBytes * 8;
		if (uiBits >= 64)
		{
			iMinimum = INT64_MIN;
			iMaximum = INT64_MAX;
		}
		else if (psField->eType == SXFT_Signed)
		{
			iMinimum = -((int64)1 << (uiBits - 1));
			iMaximum = ((int64)1 << (uiBits - 1)) - 1;
		}
		else
		{
			iMinimum = 0;
			iMaximum = ((int64)1 << uiBits) - 1;
		}
		bResult &= LoadStateInt64(pcTag, psField->szName, iMinimum, iMaximum, &iValue);
		memcpy((uint8*)pvState + psField->uiOffset, &iValue, psField->uiBytes);
	}
	return bResult;
}

