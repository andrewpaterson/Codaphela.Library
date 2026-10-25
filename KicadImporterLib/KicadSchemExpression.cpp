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
#include "KicadSchemExpression.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemExpression::Init(EKicadSchemExpressionType eType, const char* szText, size uiLength, size uiLine)
{
	meType = eType;
	mszText.Init();
	if (uiLength > 0)
	{
		mszText.AppendSubString(szText, uiLength);
	}
	mapcChildren.Init();
	muiLine = uiLine;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemExpression::Kill(void)
{
	mapcChildren.Kill();
	mszText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemExpression::AddChild(CKicadSchemExpression* pcChild)
{
	mapcChildren.Add(pcChild);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemExpression::AppendText(char c)
{
	mszText.Append(c);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EKicadSchemExpressionType CKicadSchemExpression::GetType(void)
{
	return meType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemExpression::IsList(void)
{
	return meType == KSET_List;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemExpression::IsSymbol(void)
{
	return meType == KSET_Symbol;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemExpression::IsString(void)
{
	return meType == KSET_String;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemExpression::IsAtom(void)
{
	return meType != KSET_List;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemExpression::Is(const char* szName)
{
	return (meType == KSET_List) && mszText.Equals(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemExpression::IsSymbol(const char* szText)
{
	return (meType == KSET_Symbol) && mszText.Equals(szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CKicadSchemExpression::GetText(void)
{
	return mszText.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CKicadSchemExpression::GetLine(void)
{
	return muiLine;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CKicadSchemExpression::NumChildren(void)
{
	return mapcChildren.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CKicadSchemExpression* CKicadSchemExpression::GetChild(size uiIndex)
{
	return mapcChildren.SafeGetPtr(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CKicadSchemExpression* CKicadSchemExpression::FindChild(const char* szName)
{
	return FindChild(szName, 0, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CKicadSchemExpression* CKicadSchemExpression::FindChild(const char* szName, size uiStartIndex, size* puiIndex)
{
	size					i;
	size					uiNumChildren;
	CKicadSchemExpression*	pcChild;

	uiNumChildren = mapcChildren.NumElements();
	for (i = uiStartIndex; i < uiNumChildren; i++)
	{
		pcChild = mapcChildren.GetPtr(i);
		if (pcChild->Is(szName))
		{
			SafeAssign(puiIndex, i);
			return pcChild;
		}
	}
	SafeAssign(puiIndex, uiNumChildren);
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CKicadSchemExpression::CountChildren(const char* szName)
{
	size	i;
	size	uiNumChildren;
	size	uiCount;

	uiCount = 0;
	uiNumChildren = mapcChildren.NumElements();
	for (i = 0; i < uiNumChildren; i++)
	{
		if (mapcChildren.GetPtr(i)->Is(szName))
		{
			uiCount++;
		}
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemExpression::ContainsSymbol(const char* szSymbol)
{
	size	i;
	size	uiNumChildren;

	uiNumChildren = mapcChildren.NumElements();
	for (i = 0; i < uiNumChildren; i++)
	{
		if (mapcChildren.GetPtr(i)->IsSymbol(szSymbol))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CKicadSchemExpression::GetAtomText(size uiIndex)
{
	CKicadSchemExpression*	pcChild;

	pcChild = mapcChildren.SafeGetPtr(uiIndex);
	if ((pcChild == NULL) || !pcChild->IsAtom())
	{
		return NULL;
	}
	return pcChild->GetText();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemExpression::GetDouble(size uiIndex, float64* pfValue)
{
	CKicadSchemExpression*	pcChild;

	pcChild = mapcChildren.SafeGetPtr(uiIndex);
	if ((pcChild == NULL) || !pcChild->IsSymbol())
	{
		return false;
	}
	return ParseKicadSchemDouble(pcChild->GetText(), pfValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemExpression::GetInt(size uiIndex, int32* piValue)
{
	CKicadSchemExpression*	pcChild;

	pcChild = mapcChildren.SafeGetPtr(uiIndex);
	if ((pcChild == NULL) || !pcChild->IsSymbol())
	{
		return false;
	}
	return ParseKicadSchemInt(pcChild->GetText(), piValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CKicadSchemExpression::GetChildText(const char* szName)
{
	CKicadSchemExpression*	pcChild;

	pcChild = FindChild(szName);
	if (pcChild == NULL)
	{
		return NULL;
	}
	return pcChild->GetAtomText(0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemExpression::GetChildDouble(const char* szName, float64* pfValue)
{
	CKicadSchemExpression*	pcChild;

	pcChild = FindChild(szName);
	if (pcChild == NULL)
	{
		return false;
	}
	return pcChild->GetDouble(0, pfValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemExpression::GetChildInt(const char* szName, int32* piValue)
{
	CKicadSchemExpression*	pcChild;

	pcChild = FindChild(szName);
	if (pcChild == NULL)
	{
		return false;
	}
	return pcChild->GetInt(0, piValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemExpression::GetChildYesNo(const char* szName, bool* pbValue)
{
	CKicadSchemExpression*	pcChild;
	char*					szValue;

	//KiCad 6 wrote flags as an empty list, (fields_autoplaced), and later versions as (fields_autoplaced yes).
	pcChild = FindChild(szName);
	if (pcChild == NULL)
	{
		return false;
	}

	if (pcChild->NumChildren() == 0)
	{
		*pbValue = true;
		return true;
	}

	szValue = pcChild->GetAtomText(0);
	if (szValue == NULL)
	{
		return false;
	}
	else if (StringCompare(szValue, "yes") == 0)
	{
		*pbValue = true;
		return true;
	}
	else if (StringCompare(szValue, "no") == 0)
	{
		*pbValue = false;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemExpression::Print(CChars* psz)
{
	size	i;
	size	uiNumChildren;
	char*	szText;
	char	c;

	if (meType == KSET_List)
	{
		psz->Append('(');
		psz->Append(mszText.Text());
		uiNumChildren = mapcChildren.NumElements();
		for (i = 0; i < uiNumChildren; i++)
		{
			psz->Append(' ');
			mapcChildren.GetPtr(i)->Print(psz);
		}
		psz->Append(')');
	}
	else if (meType == KSET_String)
	{
		psz->Append('"');
		szText = mszText.Text();
		for (i = 0; szText[i] != '\0'; i++)
		{
			c = szText[i];
			if (c == '"')
			{
				psz->Append("\\\"");
			}
			else if (c == '\\')
			{
				psz->Append("\\\\");
			}
			else if (c == '\n')
			{
				psz->Append("\\n");
			}
			else
			{
				psz->Append(c);
			}
		}
		psz->Append('"');
	}
	else
	{
		psz->Append(mszText.Text());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ParseKicadSchemDouble(const char* szText, float64* pfValue)
{
	size		i;
	bool		bNegative;
	uint64		ulliMantissa;
	int32		iExponent;
	int32		iExplicitExponent;
	bool		bNegativeExponent;
	bool		bDigits;
	float64		fValue;
	float64		fScale;

	//CTextParser::GetFloat loses the sign of numbers between -1 and 0 (-0.635), so KiCad's numbers are parsed here.
	//The digits are accumulated as an integer and scaled once so that 1.27 is the closest double to 1.27.
	i = 0;
	bNegative = false;
	if ((szText[i] == '-') || (szText[i] == '+'))
	{
		bNegative = szText[i] == '-';
		i++;
	}

	ulliMantissa = 0;
	iExponent = 0;
	bDigits = false;
	while ((szText[i] >= '0') && (szText[i] <= '9'))
	{
		if (ulliMantissa < 100000000000000000LL)
		{
			ulliMantissa = ulliMantissa * 10 + (szText[i] - '0');
		}
		else
		{
			iExponent++;
		}
		bDigits = true;
		i++;
	}

	if (szText[i] == '.')
	{
		i++;
		while ((szText[i] >= '0') && (szText[i] <= '9'))
		{
			if (ulliMantissa < 100000000000000000LL)
			{
				ulliMantissa = ulliMantissa * 10 + (szText[i] - '0');
				iExponent--;
			}
			bDigits = true;
			i++;
		}
	}

	if (!bDigits)
	{
		return false;
	}

	if ((szText[i] == 'e') || (szText[i] == 'E'))
	{
		i++;
		bNegativeExponent = false;
		if ((szText[i] == '-') || (szText[i] == '+'))
		{
			bNegativeExponent = szText[i] == '-';
			i++;
		}

		if ((szText[i] < '0') || (szText[i] > '9'))
		{
			return false;
		}

		iExplicitExponent = 0;
		while ((szText[i] >= '0') && (szText[i] <= '9'))
		{
			if (iExplicitExponent < 10000)
			{
				iExplicitExponent = iExplicitExponent * 10 + (szText[i] - '0');
			}
			i++;
		}
		iExponent += bNegativeExponent ? -iExplicitExponent : iExplicitExponent;
	}

	if (szText[i] != '\0')
	{
		return false;
	}

	fScale = 1.0;
	if (iExponent < 0)
	{
		for (; iExponent < 0; iExponent++)
		{
			fScale *= 10.0;
		}
		fValue = (float64)ulliMantissa / fScale;
	}
	else
	{
		for (; iExponent > 0; iExponent--)
		{
			fScale *= 10.0;
		}
		fValue = (float64)ulliMantissa * fScale;
	}

	*pfValue = bNegative ? -fValue : fValue;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ParseKicadSchemInt(const char* szText, int32* piValue)
{
	size	i;
	bool	bNegative;
	int64	lliValue;

	i = 0;
	bNegative = false;
	if ((szText[i] == '-') || (szText[i] == '+'))
	{
		bNegative = szText[i] == '-';
		i++;
	}

	if ((szText[i] < '0') || (szText[i] > '9'))
	{
		return false;
	}

	lliValue = 0;
	while ((szText[i] >= '0') && (szText[i] <= '9'))
	{
		lliValue = lliValue * 10 + (szText[i] - '0');
		if (lliValue > 0x7FFFFFFFLL)
		{
			return false;
		}
		i++;
	}

	if (szText[i] != '\0')
	{
		return false;
	}

	*piValue = (int32)(bNegative ? -lliValue : lliValue);
	return true;
}

