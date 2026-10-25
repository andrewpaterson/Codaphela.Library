/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdio.h>
#include <stdlib.h>
#include "BaseLib/StringHelper.h"
#include "ComponentProperties.h"


//////////////////////////////////////////////////////////////////////////
//
// The label is the name made readable.
//
//////////////////////////////////////////////////////////////////////////
void SComponentProperty::Init(const char* szName, EComponentPropertyType eType, const char* szValue)
{
	this->szName.Init(szName);
	JavaNameToHumanReadable(&szLabel, szName);
	szUnit.Init();
	this->eType = eType;
	this->szValue.Init(szValue);
	aszChoices.Init();
	fMinimum = 0;
	fMaximum = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SComponentProperty::Kill(void)
{
	aszChoices.Kill();
	szValue.Kill();
	szUnit.Kill();
	szLabel.Kill();
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentProperties::Init(void)
{
	masProperties.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentProperties::Kill(void)
{
	size	i;

	for (i = 0; i < masProperties.NumElements(); i++)
	{
		masProperties.Get(i)->Kill();
	}
	masProperties.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SComponentProperty* CComponentProperties::Add(const char* szName, EComponentPropertyType eType, const char* szValue)
{
	SComponentProperty*	psProperty;

	psProperty = masProperties.Add();
	psProperty->Init(szName, eType, szValue);
	return psProperty;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentProperties::AddText(const char* szName, const char* szValue)
{
	Add(szName, CPT_Text, (szValue != NULL) ? szValue : "");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentProperties::AddInteger(const char* szName, int64 iValue, int64 iMinimum, int64 iMaximum)
{
	SComponentProperty*	psProperty;
	char				szValue[32];

	sprintf(szValue, "%lld", (long long)iValue);
	psProperty = Add(szName, CPT_Integer, szValue);
	psProperty->fMinimum = (float64)iMinimum;
	psProperty->fMaximum = (float64)iMaximum;
}


//////////////////////////////////////////////////////////////////////////
//
// The value is in the unit, as Logicim's float editors show theirs.
//
//////////////////////////////////////////////////////////////////////////
void CComponentProperties::AddFloat(const char* szName, const char* szUnit, float64 fValue, float64 fMinimum, float64 fMaximum)
{
	SComponentProperty*	psProperty;
	char				szValue[64];

	sprintf(szValue, "%g", fValue);
	psProperty = Add(szName, CPT_Float, szValue);
	psProperty->szUnit.Set(szUnit);
	psProperty->fMinimum = fMinimum;
	psProperty->fMaximum = fMaximum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentProperties::AddBoolean(const char* szName, bool bValue)
{
	Add(szName, CPT_Boolean, bValue ? "true" : "false");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentProperties::AddChoice(const char* szName, const char** aszChoices, size uiNumChoices, size uiChosen)
{
	SComponentProperty*	psProperty;
	size				i;

	psProperty = Add(szName, CPT_Choice, (uiChosen < uiNumChoices) ? aszChoices[uiChosen] : "");
	for (i = 0; i < uiNumChoices; i++)
	{
		psProperty->aszChoices.Add(aszChoices[i]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentProperties::AddChoice(const char* szName, CArrayChars* paszChoices, const char* szChosen)
{
	SComponentProperty*	psProperty;
	size				i;

	psProperty = Add(szName, CPT_Choice, (szChosen != NULL) ? szChosen : "");
	for (i = 0; i < paszChoices->NumElements(); i++)
	{
		psProperty->aszChoices.Add(paszChoices->GetText(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentProperties::AddDivider(const char* szName)
{
	Add(szName, CPT_Divider, "");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CComponentProperties::NumProperties(void) { return masProperties.NumElements(); }
SComponentProperty* CComponentProperties::Get(size uiIndex) { return masProperties.SafeGet(uiIndex); }


//////////////////////////////////////////////////////////////////////////
//
// NULL if there's no property of the name.
//
//////////////////////////////////////////////////////////////////////////
SComponentProperty* CComponentProperties::Get(const char* szName)
{
	size				i;
	SComponentProperty*	psProperty;

	for (i = 0; i < masProperties.NumElements(); i++)
	{
		psProperty = masProperties.Get(i);
		if (psProperty->szName.Equals(szName))
		{
			return psProperty;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Fails if there's no property of the name.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentProperties::SetValue(const char* szName, const char* szValue)
{
	SComponentProperty*	psProperty;

	psProperty = Get(szName);
	if (psProperty == NULL)
	{
		return false;
	}
	psProperty->szValue.Set(szValue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CComponentProperties::GetText(const char* szName, const char* szDefault)
{
	SComponentProperty*	psProperty;

	psProperty = Get(szName);
	return (psProperty != NULL) ? psProperty->szValue.Text() : szDefault;
}


//////////////////////////////////////////////////////////////////////////
//
// Clamped to the property's minimum and maximum.
//
//////////////////////////////////////////////////////////////////////////
int64 CComponentProperties::GetInteger(const char* szName, int64 iDefault)
{
	SComponentProperty*	psProperty;
	char*				szEnd;
	int64				iValue;

	psProperty = Get(szName);
	if ((psProperty == NULL) || psProperty->szValue.Empty())
	{
		return iDefault;
	}
	iValue = (int64)strtoll(psProperty->szValue.Text(), &szEnd, 0);
	while (*szEnd == ' ')
	{
		szEnd++;
	}
	if (*szEnd != '\0')
	{
		return iDefault;
	}
	if (iValue < (int64)psProperty->fMinimum)
	{
		return (int64)psProperty->fMinimum;
	}
	if (iValue > (int64)psProperty->fMaximum)
	{
		return (int64)psProperty->fMaximum;
	}
	return iValue;
}


//////////////////////////////////////////////////////////////////////////
//
// Clamped to the property's minimum and maximum.
//
//////////////////////////////////////////////////////////////////////////
float64 CComponentProperties::GetFloat(const char* szName, float64 fDefault)
{
	SComponentProperty*	psProperty;
	char*				szEnd;
	float64				fValue;

	psProperty = Get(szName);
	if ((psProperty == NULL) || psProperty->szValue.Empty())
	{
		return fDefault;
	}
	fValue = strtod(psProperty->szValue.Text(), &szEnd);
	while (*szEnd == ' ')
	{
		szEnd++;
	}
	if (*szEnd != '\0')
	{
		return fDefault;
	}
	if (fValue < psProperty->fMinimum)
	{
		return psProperty->fMinimum;
	}
	if (fValue > psProperty->fMaximum)
	{
		return psProperty->fMaximum;
	}
	return fValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponentProperties::GetBoolean(const char* szName, bool bDefault)
{
	SComponentProperty*	psProperty;

	psProperty = Get(szName);
	if (psProperty == NULL)
	{
		return bDefault;
	}
	return psProperty->szValue.Equals("true");
}


//////////////////////////////////////////////////////////////////////////
//
// The index of the chosen choice.
//
//////////////////////////////////////////////////////////////////////////
size CComponentProperties::GetChoice(const char* szName, size uiDefault)
{
	SComponentProperty*	psProperty;
	size				i;

	psProperty = Get(szName);
	if (psProperty == NULL)
	{
		return uiDefault;
	}
	for (i = 0; i < psProperty->aszChoices.NumElements(); i++)
	{
		if (psProperty->szValue.Equals(psProperty->aszChoices.GetText(i)))
		{
			return i;
		}
	}
	return uiDefault;
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's javaNameToHumanReadable: "explicitPowerPorts" is "Explicit
// power ports".
//
//////////////////////////////////////////////////////////////////////////
void JavaNameToHumanReadable(CChars* pszDest, const char* szName)
{
	size	i;
	size	uiLength;
	char	c;
	char	cNext;
	char	cPrevious;

	pszDest->Init();
	uiLength = StrLen(szName);
	for (i = 0; i < uiLength; i++)
	{
		c = (szName[i] == '_') ? ' ' : szName[i];
		cNext = (i + 1 < uiLength) ? szName[i + 1] : 'Z';
		cPrevious = (i > 0) ? szName[i - 1] : 'z';
		if (i == 0)
		{
			pszDest->Append((char)(((c >= 'a') && (c <= 'z')) ? c - 'a' + 'A' : c));
		}
		else if ((c >= 'A') && (c <= 'Z') && (cNext >= 'a') && (cNext <= 'z'))
		{
			if (cPrevious != ' ')
			{
				pszDest->Append(' ');
			}
			pszDest->Append((char)(c - 'A' + 'a'));
		}
		else if ((cPrevious >= 'a') && (cPrevious <= 'z') && (c >= 'A') && (c <= 'Z'))
		{
			pszDest->Append(' ');
			pszDest->Append(c);
		}
		else
		{
			pszDest->Append(c);
		}
	}
}

