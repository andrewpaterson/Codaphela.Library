/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "DebugOutput.h"
#include "PointerRemapper.h"
#include "IntegerHelper.h"
#include "ArrayChars.h"
#include "StringHelper.h"
#include "Newline.h"
#include "FileWriter.h"
#include "FileReader.h"
#include "FileUtil.h"
#include "NullAllocator.h"
#include "FloatPrinter.h"
#include "Chars.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareChars(const void* arg1, const void* arg2)
{
	CChars*	szString1;
	CChars*	szString2;

	szString1 = (CChars*)arg1;
	szString2 = (CChars*)arg2;

	return szString1->Compare(szString2);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareCharsIgnoreCase(const void* arg1, const void* arg2)
{
	CChars*	szString1;
	CChars*	szString2;

	szString1 = (CChars*)arg1;
	szString2 = (CChars*)arg2;

	return szString1->CompareIgnoreCase(szString2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::_Init(void)
{
	//This makes it safe to call kill.
	//It does not make 'this' usable.  It will - rightly - NULL pointer.
	memset(this, 0, sizeof(CChars));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::InitLength(size iLength)
{
	if (iLength != 0)
	{
		mcText.Init();
		mcText.Resize(iLength + 1);
		mcText.SetValue(iLength, '\0');
		return this;
	}
	else
	{
		InitEmpty();
		return this;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Init(void)
{
	InitEmpty();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Init(const char* szString)
{
	mcText.Init();
	if (!StrEmpty(szString))
	{
		Set(szString);
	}
	else
	{
		SetEmpty();
	}
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Init(CChars szString, size iStartInclusive, size iEndExclusive)
{
	if (iEndExclusive > iStartInclusive)
	{
		mcText.Init();
		AppendSubString(szString, iStartInclusive, iEndExclusive);
		CleanIfEmpty();
	}
	else
	{
		InitEmpty();
	}
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Init(const char* szString, size iStartInclusive, size iEndExclusive)
{
	if (iEndExclusive > iStartInclusive)
	{
		mcText.Init();
		AppendSubString(szString, iStartInclusive, iEndExclusive);
		CleanIfEmpty();
	}
	else
	{
		InitEmpty();
	}
	return this;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Init(const char* szString, size iStartInclusive)
{
	mcText.Init();
	Set(&szString[iStartInclusive]);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Init(CChars szString)
{
	if (!szString.Empty())
	{
		mcText.Init();
		mcText.Copy(&(szString.mcText));
	}
	else
	{
		InitEmpty();
	}

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Init(CChars* psz)
{
	if (psz && !psz->Empty())
	{
		mcText.Init();
		mcText.Copy(&(psz->mcText));
	}
	else
	{
		InitEmpty();
	}
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Init(char cPadCharacter, size iNumber)
{
	if (iNumber != 0)
	{
		mcText.Init();
		Append(cPadCharacter, iNumber);
	}
	else
	{
		InitEmpty();
	}
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Init(CExternalString* pcString)
{
	return Init(pcString->msz, 0, pcString->miLen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::InitList(const char* szFirst, ...)
{
	InitEmpty();

	va_list			vaMarker;
	const char*		pc;
	size				iCount;

	if (szFirst)
	{
		iCount = 0;
		pc = szFirst;

		va_start(vaMarker, szFirst);
		while (pc)
		{
			Append(pc);
			iCount++;
			pc = va_arg(vaMarker, char*);
		}
		va_end(vaMarker);
	}

	CleanIfEmpty();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::InitList(CChars* szFirst, ...)
{
	InitEmpty();

	va_list		vaMarker;
	CChars*		pc;
	size			iCount;

	if (szFirst)
	{
		iCount = 0;
		pc = szFirst;

		va_start(vaMarker, szFirst);
		while (pc && pc->Length() != 0)
		{
			Append(pc->Text());
			iCount++;
			pc = va_arg(vaMarker, CChars*);
		}
		va_end(vaMarker);
	}
	CleanIfEmpty();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChars::InitData2(const char* szData, size iDataLength)
{
	InitEmpty();

	return AppendData2(szData, iDataLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Kill(void)
{
	if (!IsFakeEmpty())
	{
		mcText.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::DumpKill(void)
{
	Dump();
	Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Fake(const char* szString)
{
	if (szString)
	{
		mcText.Fake((char*)szString, strlen(szString) + 1);
	}
	else
	{
		InitEmpty();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Fake(const char* szString, size iStartInclusive, size iEndExclusive)
{
	char* pcPosition;

	if (iEndExclusive > iStartInclusive)
	{
		pcPosition = (char*)RemapSinglePointer(szString, iStartInclusive);
		mcText.Fake(pcPosition, iEndExclusive - iStartInclusive + 1);
	}
	else
	{
		InitEmpty();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::ReInit(void)
{
	Kill();
	return Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChars::Length(void)
{
	//Don't include the zero char at the end of the string.
	if (mcText.IsEmpty() || IsFakeEmpty())
	{
		return 0;
	}
	return mcText.NumElements() - 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Set(CChars szString)
{
	size uiLen;

	uiLen = szString.Length() + 1;
	if (uiLen > 1)
	{
		SetNonNull(szString.Text(), uiLen);
	}
	else
	{
		SetEmpty();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Set(CChars* psz)
{
	size uiLen;

	uiLen = psz->Length() + 1;
	if (uiLen > 1)
	{
		SetNonNull(psz->Text(), uiLen);
	}
	else
	{
		SetEmpty();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Set(const char* szString)
{
	size uiLen;

	if (!StrEmpty(szString))
	{
		uiLen = (strlen(szString)) + 1;
		SetNonNull(szString, uiLen);
	}
	else
	{
		SetEmpty();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::SetNonNull(const char* szString, size uiLen)
{
	if (!IsFakeEmpty())
	{
		mcText.Resize(uiLen);
	}
	else
	{
		Unfake();
		mcText.Resize(uiLen);
	}
	memcpy(mcText.GetData(), szString, uiLen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::SetEmpty(void)
{
	if (!IsFakeEmpty())
	{
		mcText.Resize(0);
		InitEmpty();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::SetChar(size iIndex, char c)
{
	if (iIndex < Length())
	{
		mcText.SetValue(iIndex, c);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Append(const char* szString)
{
	size	uiLen;
	char*	pcPosition;

	if (!StrEmpty(szString))
	{
		uiLen = strlen(szString);
		pcPosition = PrivateGrow(uiLen);
		memcpy(pcPosition, szString, uiLen);
		pcPosition[uiLen] = '\0';
	}

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Append(CExternalString* pcString)
{
	char*	pcPosition;

	if (pcString && pcString->miLen > 0)
	{
		pcPosition = PrivateGrow(pcString->miLen);
		memcpy(pcPosition, pcString->msz, pcString->miLen);
		pcPosition[pcString->miLen] = '\0';
	}

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Append(const char* szString, size uiLen)
{
	char*	pcPosition;
	char*	pcZero;

	if (szString && uiLen > 0)
	{
		pcPosition = PrivateGrow(uiLen);
		memcpy(pcPosition, szString, uiLen);
		pcZero = mcText.Tail();
		*pcZero = '\0';
	}

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Append(char cPadCharacter)
{
	char* pcReplace;

	UnfakeIfFakeEmpty();

	pcReplace = mcText.Tail();
	*pcReplace = cPadCharacter;
	mcText.Add(0);

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Append(CChars szString)
{
	Append(szString.Text());
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Append(CChars* psz)
{
	Append(psz->Text());
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Append(int32 i)
{
	char szString[32];

	IntToString(szString, 32, i, 10);
	return Append(szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Append(int32 i, uint16 iBase)
{
	char szString[32];

	IntToString(szString, 32, i, iBase);
	return Append(szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Append(uint64 ulli)
{
	char szString[64];

	LongToString(szString, 64, ulli);
	return Append(szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Append(int64 lli)
{
	char szString[64];

	LongToString(szString, 64, lli);
	return Append(szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Append(uint32 ui)
{
	char szString[32];

	IntToString(szString, 32, ui);
	return Append(szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Append(float f, size iMaxDecimals, bool bAppendF)
{
	char szString[32];

	FloatToString(szString, 32, f, iMaxDecimals, bAppendF);
	return  Append(szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Append(double d, size iMaxDecimals, bool bAppendD)
{
	char	szString[128];

	DoubleToString(szString, 128, d, iMaxDecimals, bAppendD);
	return Append(szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::AppendQuoted(const char* szString)
{
	Append('"');
	Append(szString);
	Append('"');

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::AppendQuoted(char c)
{
	Append('\'');
	Append(c);
	Append('\'');

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::AppendNewLine(void)
{
	return Append("\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::AppendNewLine(CChars szString)
{
	Append(&szString);
	return AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char GetNumberAs1DigitHex(size nybble)
{
	if (nybble <= 15)
	{
		return gszDigits[nybble];
	}
	return '?';
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::AppendHexHiLo(void* pv, size iNumBytes)
{
	size	i;
	char*	pb;
	int8	nibble;
	char	c;

	if (iNumBytes != 0)
	{
		i = iNumBytes;
		do
		{
			i--;
			pb = (char*)RemapSinglePointer(pv, i);
			nibble = GetHighNybble(*pb);
			c = GetNumberAs1DigitHex(nibble);
			Append(c);
			nibble = GetLowNybble(*pb);
			c = GetNumberAs1DigitHex(nibble);
			Append(c);
		}
		while (i != 0);
	}

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::AppendHexLoHi(void* pv, size iNumBytes)
{
	size	i;
	char*	pb;
	int8	nibble;
	char	c;

	if (iNumBytes != 0)
	{
		for (i = 0; i < iNumBytes; i++)
		{
			pb = (char*)RemapSinglePointer(pv, i);
			nibble = GetHighNybble(*pb);
			c = GetNumberAs1DigitHex(nibble);
			Append(c);
			nibble = GetLowNybble(*pb);
			c = GetNumberAs1DigitHex(nibble);
			Append(c);
		}
	}

	return this;
}


///////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::AppendBool(bool bValue)
{
	return AppendBool(bValue, "True", "False");
}


/////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::AppendBool(bool bValue, const char* szTrue, const char* szFalse)
{
	if (bValue)
	{
		return Append(szTrue);
	}
	else
	{
		return Append(szFalse);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Insert(size iPos, char c)
{
	if (iPos >= mcText.NumElements())
	{
		return;
	}

	UnfakeIfFakeEmpty();

	mcText.InsertAt(c, iPos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Insert(size iPos, const char* szString)
{
	char*	pcNew;
	size	uiInsertLen;

	if (StrEmpty(szString))
	{
		return;
	}

	UnfakeIfFakeEmpty();
	uiInsertLen = strlen(szString);
	pcNew = mcText.InsertNumAt(uiInsertLen, iPos);
	if (pcNew)
	{
		memcpy(pcNew, szString, uiInsertLen);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Insert(size iPos, CChars* pszString)
{
	char*	pcNew;
	size uiInsertLen;

	if (pszString == NULL || pszString->Empty())
	{
		return;
	}

	UnfakeIfFakeEmpty();
	uiInsertLen = pszString->Length();
	pcNew = mcText.InsertNumAt(uiInsertLen, iPos);
	if (pcNew)
	{
		memcpy(pcNew, pszString->Text(), uiInsertLen);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Minimize(void)
{
	if (!IsFakeEmpty())
	{
		mcText.Finalise();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CChars::Text(void)
{
	return mcText.GetData();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CChars::Text(size iIndex)
{
	if (iIndex < Length())
	{
		return mcText.Get(iIndex);
	}
	else
	{
		return mcText.Tail();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChars::Empty(void)
{
	if (Length() == 0)
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::AppendList(const char* szFirst, ...)
{
	va_list			vaMarker;
	const char*		pc;
	size				iCount;

	iCount = 0;
	pc = szFirst;

	va_start(vaMarker, szFirst);
	while (pc != NULL)
	{
		Append(pc);
		iCount++;
		pc = va_arg(vaMarker, char*);
	}
	va_end(vaMarker);

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CChars::PrivateGrow(size iNumberOfCharacters)
{
	char*	pszPosition;
	size	iPosition;

	if (mcText.IsEmpty())
	{
		if (iNumberOfCharacters != 0)
		{
			mcText.AddNum(iNumberOfCharacters + 1);  //To include trailing zero.
		}
		else
		{
			SetEmpty();
		}
		return mcText.GetData();
	}
	else
	{
		if (iNumberOfCharacters != 0)
		{
			if (IsFakeEmpty())
			{
				Unfake();
			}
			iPosition = mcText.AddNum(iNumberOfCharacters);
			pszPosition = (char*)RemapSinglePointer(mcText.GetData(), iPosition - 1);
			return pszPosition;
		}
		else
		{
			SetEmpty();
			return mcText.GetData();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Append(char cPadCharacter, size iNumber)
{
	char*	pcPosition;

	if ((iNumber != 0) && (iNumber != ARRAY_ELEMENT_NOT_FOUND))
	{
		pcPosition = PrivateGrow(iNumber);
		memset(pcPosition, cPadCharacter, iNumber);
		pcPosition = mcText.Tail();
		*pcPosition = '\0';
	}

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::LeftAlign(const char* szString, char cPadCharacter, size uiWidth)
{
	size	uiLen;

	uiLen = strlen(szString);
	if (uiLen != 0)
	{
		uiLen = strlen(szString);
		if (uiLen <= uiWidth)
		{
			Append(szString);
			Append(cPadCharacter, uiWidth - uiLen);
		}
		else
		{
			AppendSubString(szString, 0, uiWidth);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::RightAlign(const char* szString, char cPadCharacter, size uiWidth)
{
	size	uiLen;

	uiLen = strlen(szString);
	if (uiLen != 0)
	{
		if (uiLen <= uiWidth)
		{
			Append(cPadCharacter, uiWidth - uiLen);
			Append(szString);
		}
		else
		{
			AppendSubString(szString, 0, uiWidth);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::LeftAlign(CChars szString, char cPadCharacter, size uiWidth)
{
	if (szString.Length() <= uiWidth)
	{
		Append(szString);
		Append(cPadCharacter, uiWidth - szString.Length());
	}
	else
	{
		AppendSubString(szString, 0, uiWidth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::RightAlign(CChars szString, char cPadCharacter, size uiWidth)
{
	if (szString.Length() <= uiWidth)
	{
		Append(cPadCharacter, uiWidth - szString.Length());
		Append(szString);
	}
	else
	{
		AppendSubString(szString, 0, uiWidth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::RightAlign(char cPadCharacter, size uiWidth)
{
	size	iLength;
	char*	pcDest;
	size	iOldWidth;

	if (Length() < uiWidth)
	{
		iLength = uiWidth - Length();
		iOldWidth = Length();
		PrivateGrow(iLength);
		pcDest = (char*)RemapSinglePointer(mcText.GetData(), iLength);
		memmove(pcDest, mcText.GetData(), iOldWidth);
		memset(mcText.GetData(), cPadCharacter, iLength);
		(*mcText.Tail()) = '\0';
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::AppendSubString(const char* szString, size iLength)
{
	char*	pcPosition;

	if ((szString) && (iLength != 0))
	{
		pcPosition = PrivateGrow(iLength);
		memcpy(pcPosition, szString, iLength);
		(*mcText.Tail()) = '\0';
	}

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::AppendSubString(CChars szString, size iStartInclusive, size iEndExclusive)
{
	const char* pcPosition;

	pcPosition = (char*)RemapSinglePointer((void*)szString.Text(), iStartInclusive);
	return AppendSubString(pcPosition, iEndExclusive-iStartInclusive);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::AppendSubString(const char* szString, size iStartInclusive, size iEndExclusive)
{
	const char* pcPosition;

	pcPosition = (char*)RemapSinglePointer((void*)szString, iStartInclusive);
	return AppendSubString(pcPosition, iEndExclusive-iStartInclusive);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::AppendSubString(const char* szStartInclusive, const char* szEndExclusive)
{
	return AppendSubString(szStartInclusive, (size)(szEndExclusive-szStartInclusive));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::RemoveLastCharacter(void)
{
	char*	pcPosition;

	if (Length() != 0)
	{
		mcText.RemoveTail();
		if (mcText.NumElements() > 1)
		{
			pcPosition = mcText.Tail();
			*pcPosition = '\0';
		}
		else
		{
			SetEmpty();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::RemoveFromStart(size iNumChars)
{
	Remove(0, iNumChars);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::RemoveFromEnd(size iNumChars)
{
	size iLength;

	iLength = Length();
	if (iNumChars >= iLength)
	{
		SetEmpty();
	}

	Remove(iLength - iNumChars, iLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Remove(size iStartInclusive, size iEndExclusive)
{
	size	iLength;

	if (!Empty())
	{
		iLength = Length();
		if (iEndExclusive > iLength)
		{
			iEndExclusive = iLength;
		}
		mcText.RemoveRange(iStartInclusive, iEndExclusive);
		CleanIfEmpty();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::RemoveEnd(size iIndex)
{
	size	iToRemove;

	iToRemove = Length() - iIndex;
	if (iToRemove != 0)
	{
		if (iIndex != 0)
		{
			mcText.RemoveRange(iIndex, Length());
		}
		else
		{
			SetEmpty();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::RemoveCharacter(size iPos)
{
	if (iPos < Length())
	{
		mcText.RemoveAt(iPos, true);
		CleanIfEmpty();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Split(CArrayChars* aszDest, char cSplitter)
{
	size	iPos;
	char*	pszPos;
	size	iMax;

	pszPos = Text();
	iMax = Length();
	for (;;)
	{
		iPos = FindFirstByte((uint8*)pszPos, cSplitter, iMax);
		if (iPos != ARRAY_ELEMENT_NOT_FOUND)
		{
			aszDest->Add(pszPos, 0, iPos);
			iPos++;
			pszPos = (char*)RemapSinglePointer((void*)pszPos, iPos);
			iMax -= iPos;
		}
		else
		{
			aszDest->Add(pszPos, 0, iMax);
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::SplitLines(CArrayChars* aszDest)
{
	//This assumes new lines are passified.
	Split(aszDest, '\n');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChars::Equals(const char* szString)
{
	if (szString)
	{
		if (StringCompare(Text(), szString) == 0)
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
bool CChars::Equals(const char* szString, size uiLen)
{
	if (uiLen == Length())
	{
		if (memcmp(Text(), szString, uiLen) == 0)
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
bool CChars::Equals(CChars szString)
{
	return Equals(szString.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChars::Equals(CChars* pszString)
{
	return Equals(pszString->Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChars::EqualsIgnoreCase(const char* szString)
{
	if (StringInsensitiveCompare(Text(), szString) == 0)
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChars::EqualsIgnoreCase(CChars szString)
{
	return EqualsIgnoreCase(szString.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChars::Contains(const char* szString)
{
	char*	pc;

	if (!szString) 
	{
		return false;
	}

	pc = strstr(Text(), szString);
	if (pc)
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChars::ContainsIgnoreCase(const char* szString)
{
	char*	pc;

	pc = StrIStr(Text(), szString);
	if (pc)
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChars::Occurrences(const char* szString)
{
	size	iCount;
	size	iIndex;
	size	iFindLen;

	iFindLen = strlen(szString);
	if (iFindLen == 0)
	{
		return 0;
	}

	iIndex = Find(0, szString);
	if (iIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return 0;
	}

	iCount = 0;
	for (;;)
	{
		iCount++;
		iIndex += iFindLen;
		iIndex = Find(iIndex, szString);
		if (iIndex == ARRAY_ELEMENT_NOT_FOUND)
		{
			return iCount;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChars::StartsWith(const char* szString)
{
	if (szString == NULL)
	{
		return false;
	}

	return SubStringEquals(0, szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChars::StartsWithIgnoreCase(const char* szString)
{
	if (szString == NULL)
	{
		return false;
	}

	return SubStringEqualsIgnoreCase(0, szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChars::EndsWith(const char* szString)
{
	size	uiLen;

	if (szString == NULL)
	{
		return false;
	}

	uiLen = strlen(szString);
	if (uiLen > Length())
	{ 
		return false;
	}

	return SubStringEquals(Length() - uiLen, szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChars::EndsWith(char c)
{
	size		iLength;

	iLength = Length();
	if (iLength != 0)
	{
		if (Text()[iLength - 1] == c)
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
bool CChars::EndsWithIgnoreCase(const char* szString)
{
	size	uiLen;

	if (szString == NULL)
	{
		return false;
	}

	uiLen = strlen(szString);
	if (uiLen > Length())
	{
		return false;
	}

	return SubStringEqualsIgnoreCase(Length() - strlen(szString), szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChars::SubStringEquals(size iStart, const char* szString)
{
	size	i;
	size	j;
	size	uiLen;
	size	iSubLen;

	uiLen = Length();
	if (uiLen == 0)
	{
		iSubLen = strlen(szString);
		if (iSubLen != 0)
		{
			return false;
		}
		else
		{
			return iStart == 0;
		}
	}

	if (iStart >= uiLen)
	{
		return false;
	}

	for (i = iStart, j = 0; i < uiLen; i++, j++)
	{
		if (szString[j] != 0)
		{
			if (mcText.GetValue(i) != szString[j])
			{
				return false;
			}
		}
		else
		{
			break;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChars::SubStringEqualsIgnoreCase(size iStart, const char* szString)
{
	size	i;
	size	j;
	size	uiLen;
	size    c1;
	size    c2;

	uiLen = Length();
	for (i = iStart, j = 0; i < uiLen; i++, j++)
	{
		if (szString[j] != 0)
		{
			c1 = ToLower(mcText.GetValue(i));
			c2 = ToLower(szString[j]);
			if (c1 != c2)
			{
				return false;
			}
		}
		else
		{
			break;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChars::FindFromEnd(const char* szString)
{
	size iOtherLen;

	iOtherLen = strlen(szString);
	return FindFromEnd(Length() - iOtherLen, szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChars::FindFromEnd(size iIndex, const char* szString)
{
	size	i;

	if (iIndex > Length())
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}

	i = iIndex + 1;
	do
	{
		i--;
		if (SubStringEquals(i, szString))
		{
			return i;
		}
	}
	while (i != 0);
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChars::FindFromEnd(char c)
{
	if (mcText.IsEmpty() || IsFakeEmpty())
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	
	return FindFromEnd(Length() - 1, c);

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChars::FindFromEnd(size iIndex, char c)
{
	size	i;

	i = iIndex + 1;
	do
	{
		i--;
		if (mcText.GetValue(i) == c)
		{
			return i;
		}
	}
	while (i != 0);
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChars::Find(const char* szString)
{
	return Find(0, szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChars::Find(size iIndex, const char* szString)
{
	size	iOtherLen;
	size	i;
	size	uiLen;
	size	iDiff;

	uiLen = Length();
	if (iIndex > uiLen)
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}

	iOtherLen = strlen(szString);
	if (iOtherLen > uiLen - iIndex)
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}

	iDiff = uiLen - iOtherLen;
	for (i = iIndex; i <= iDiff; i++)
	{
		if (SubStringEquals(i, szString))
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChars::Find(size iIndex, char c)
{
	size	i;
	size	uiLen;

	uiLen = Length();
	for (i = iIndex; i < uiLen; i++)
	{
		if (mcText.GetValue(i) == c)
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChars::Find(char c)
{
	return Find(0, c);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChars::FindDigit(size iIndex)
{
	size	i;
	char	c;
	size	uiLen;

	uiLen = Length();
	for (i = iIndex; i < uiLen; i++)
	{
		c = *mcText.Get(i);
		if ((c >= '0') && (c <= '9'))
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CChars::GetChar(size iIndex)
{
	char*	pc;

	pc = mcText.SafeGet(iIndex);
	if (pc)
	{
		return *pc;
	}
	return '\0';
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Difference(CArrayInt* paiNewToOldIndices, CArrayInt* paiOldToNewIndices, CChars szOldString)
{
	size	i;
	size	j;
	CChars	cTemp;
	char	cNew;
	char	cOld;
	size	uiLen;
	size	iOldLen;

	uiLen = Length();

	//This will find the index of each character in this string in the old string.

	paiOldToNewIndices->SetUsedElements(szOldString.Length());
	paiOldToNewIndices->SetArrayValues(-1);

	paiNewToOldIndices->SetUsedElements(uiLen);
	paiNewToOldIndices->SetArrayValues(-1);

	cTemp.Init(szOldString);
	iOldLen = cTemp.Length();
	for (j = 0; j < uiLen; j++)
	{
		cNew = GetChar(j);
		for (i = 0; i < iOldLen; i++)
		{
			cOld = cTemp.GetChar(i);
			if (cOld == cNew)
			{
				cTemp.SetChar(i, 1);
				paiOldToNewIndices->SetValue(i, j);
				paiNewToOldIndices->SetValue(j, i);
				break;
			}
		}
	}
	cTemp.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChars::IsWhitespace(size iPos, bool bIncludeNewLines)
{
	char	c;

	c = GetChar(iPos);
	return ::IsWhitespace(c, bIncludeNewLines);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChars::IsWhitespace(void)
{
	size		i;
	size		uiLen;

	uiLen = Length();
	for (i = 0; i < uiLen; i++)
	{
		if (!IsWhitespace(i, true))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::SetLength(size iLength)
{
	if (iLength != 0)
	{
		UnfakeIfFakeEmpty();

		//This assumes you'll fill mcText with iLengths count of non-zero characters.
		mcText.Resize(iLength + 1);
		mcText.SetValue(iLength, '\0');
	}
	else
	{
		SetEmpty();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChars::FindEndOfLeadingWhitespace(bool bIncludeNewLines)
{
	size	iLength;
	size	iStart;

	iLength = Length();
	for (iStart = 0; iStart < iLength; iStart++)
	{
		if (!IsWhitespace(iStart, bIncludeNewLines))
		{
			break;
		}
	}
	return iStart;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChars::FindStartOfTrailingWhitespace(bool bIncludeNewLines)
{
	size	iLength;
	size	iEnd;

	iLength = Length();
	if (iLength != 0)
	{
		iEnd = iLength;
		do
		{
			iEnd--;
			if (!IsWhitespace(iEnd, bIncludeNewLines))
			{
				break;
			}
		}
		while (iEnd != 0);

		return iEnd;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChars::StripWhitespace(bool bIncludeNewLines)
{
	size	iLength;
	size	iStart;
	size	iEnd;

	iStart = FindEndOfLeadingWhitespace(bIncludeNewLines);
	iEnd = FindStartOfTrailingWhitespace(bIncludeNewLines);

	if (iEnd < iStart)
	{
		SetEmpty();
	}
	else
	{
		iLength = Length();
		Remove(iEnd+1, iLength);
		Remove(0, iStart);
	}
	return iStart;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChars::Compare(CChars* szOther)
{
	return StringCompare(Text(), szOther->Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChars::CompareIgnoreCase(CChars* szOther)
{
	return StringInsensitiveCompare(Text(), szOther->Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChars::Compare(const char* szOther)
{
	return StringCompare(Text(), szOther);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChars::CompareIgnoreCase(const char* szOther)
{
	return StringInsensitiveCompare(Text(), szOther);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChars::Replace(char cFind, char cReplace)
{
	size	i;
	size	iLength;
	size	iCount;

	iLength = Length();
	iCount = 0;
	for (i = 0; i < iLength; i++)
	{
		if (mcText.GetValue(i) == cFind)
		{
			mcText.SetValue(i, cReplace);
			iCount++;
		}
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChars::Replace(const char* szFind, const char* szReplace)
{
	size	iReplaceLen;
	size	iFindLen;

	if (szFind == NULL)
	{
		return 0;
	}

	iFindLen = strlen(szFind);
	if (iFindLen == 0)
	{
		return 0;
	}

	if (szReplace == NULL)
	{
		iReplaceLen = 0;
	}
	else
	{
		iReplaceLen = strlen(szReplace);
	}

	if (iReplaceLen > iFindLen)
	{
		return ReplaceWithLonger(szFind, szReplace, iFindLen, iReplaceLen - iFindLen);
	}
	else if (iFindLen > iReplaceLen)
	{
		return ReplaceWithShorter(szFind, szReplace, iReplaceLen, iFindLen, iFindLen - iReplaceLen);
	}
	else
	{
		return ReplaceWithEqualLength(szFind, szReplace, iFindLen);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChars::ReplaceWithEqualLength(const char* szFind, const char* szReplace, size iFindLen)
{
	size		iCount;
	size		iIndex;

	iCount = 0;
	iIndex = Find(0, szFind);
	while (iIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		Overwrite(iIndex, szReplace);
		iCount++;
		iIndex = Find(iIndex + iFindLen, szFind);
	}
	return iCount;}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChars::ReplaceWithShorter(const char* szFind, const char* szReplace, size iReplaceLen, size iFindLen, size iDifference)
{
	size		iCount;
	size		iIndex;
	size		iTotalDifference;
	size		iOldIndex;
	size		iDestPos;
	size		iSize;
	size		iEnd;

	//Remember that iDifference is Positive!
	iCount = 0;
	iIndex = Find(0, szFind);
	if (iIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return 0;
	}

	iOldIndex = 0;
	iDestPos = 0;  // iDestPos will be correctly initialised after iCount loops once
	for (;;)
	{
		iSize = iIndex - iOldIndex - iDifference;

		if (iReplaceLen != 0)
		{
			if (iCount != 0)
			{
				memcpy(mcText.Get(iDestPos), mcText.Get(iOldIndex + iFindLen), iSize - 1);
			}
			Overwrite(iIndex - (iCount * iDifference), szReplace);
		}
		else
		{
			if (iCount != 0)
			{
				memcpy(mcText.Get(iDestPos), mcText.Get(iOldIndex + iFindLen), iSize);
			}
		}
		iCount++;
		iOldIndex = iIndex;
		iDestPos = iOldIndex + iReplaceLen - (iDifference * (iCount - 1));
		iIndex = Find(iIndex + iFindLen, szFind);
		if (iIndex == ARRAY_ELEMENT_NOT_FOUND)
		{
			iTotalDifference = iDifference * iCount;
			iEnd = Length() - iTotalDifference;
			iSize = Length() - iOldIndex - iDifference;
			if (iEnd != iDestPos)
			{
				if (iCount != 0)
				{
					memcpy(mcText.Get(iDestPos), mcText.Get(iOldIndex + iFindLen), iSize);
				}
			}
			break;
		}
	}

	mcText.RemoveNum(iTotalDifference);
	(*mcText.Tail()) = '\0';
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChars::ReplaceWithLonger(const char* szFind, const char* szReplace, size iFindLen, size iDifference)
{
	size		iCount;
	size		iIndex;
	size		i;
	size		iTotalDifference;
	size		iOldIndex;

	iCount = Occurrences(szFind);
	if (iCount == 0)
	{
		return 0;
	}

	iTotalDifference = iDifference * iCount;
	mcText.AddNum(iTotalDifference);
	(*mcText.Tail()) = '\0';

	iIndex = Length() - (iFindLen + iTotalDifference);
	i = iCount;
	do
	{
		i--;
		iOldIndex = iIndex;
		iIndex = FindFromEnd(iIndex, szFind);
		memcpy(mcText.Get(iIndex + iFindLen + (iDifference * (i + 1))), mcText.Get(iIndex + iFindLen), iOldIndex - iIndex);
		Overwrite(iIndex + (iDifference * i), szReplace);
		iIndex -= iFindLen;
	}
	while (i != 0);

	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Overwrite(size iIndex, const char* szReplace)
{
	size		i;
	size		j;
	size		uiLen;

	if (szReplace == NULL)
	{
		return;
	}

	if (StrEmpty(szReplace))
	{
		return;
	}

	uiLen = Length();
	for (i = iIndex, j = 0; i < uiLen; i++, j++)
	{
		if (szReplace[j] == 0)
		{
			return;
		}
		mcText.SetValue(i, szReplace[j]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Reverse(void)
{
	if (!Empty())
	{
		StrRev(mcText.GetData(), mcText.NumElements());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Clear(void)
{
	if (!IsFakeEmpty())
	{
		SetEmpty();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::AppendData(const char* szData, size iMaxLength)
{
	size	i;
	size	iLength;

	iLength = iMaxLength;
	for (i = 0; i < iMaxLength; i++)
	{
		if (szData[i] == 0)
		{
			iLength = i;
			break;
		}
	}

	return Append(szData, iLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::AppendData(const char* szData, size iDataLength, size iMaxLength)
{
	size	i;
	size	iLength;
	uint8	c;
	bool	bLastReadable;
	
	if (iDataLength > iMaxLength)
	{
		iLength = iMaxLength;
	}
	else
	{
		iLength = iDataLength;
	}

	bLastReadable = true;
	for (i = 0; i < iLength; i++)
	{
		if (!bLastReadable)
		{
			Append(",");
		}

		c = szData[i];
		if (c == 0)
		{
			if ((bLastReadable) && (i != 0))
			{
				Append("\\0");
			}
			else
			{
				Append(" 0x00");
			}
			bLastReadable = false;
		}
		else if (c == '\n')
		{
			if (bLastReadable)
			{
				Append("\\r");
			}
			else
			{
				Append(" 0x0D");
			}
			bLastReadable = false;
		}
		else if ((c >= 32) && (c <= 126))
		{
			if (!bLastReadable)
			{
				Append(' ');
			}
			bLastReadable = true;
			Append((char)c);
		}
		else
		{
			bLastReadable = false;
			Append(" 0x");
			AppendHexHiLo(&c, 1);
		}
	}

	if (iDataLength > iMaxLength)
	{
		Append("...");
	}

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChars::AppendData2(const char* szData, size iDataLength)
{
	size	i;
	size	iPrintable;
	uint8	c;
	float	fPrintable;
	CChars	szString;

	if ((szData == NULL) || (iDataLength == 0))
	{
		return false;
	}

	iPrintable = StrPrintable(szData, iDataLength);
	fPrintable = (float)iPrintable / (float)iDataLength;

	if (fPrintable >= 0.9f)
	{
		Append(szData, iDataLength);
		return false;
	}
	else
	{
		Append("0x");

		szString.Init();

		for (i = 0; i < iDataLength; i++)
		{
			c = szData[i];

			szString.Clear();
			szString.Append((int32)c, (uint16)16);
			szString.RightAlign('0', 2);

			Append(szString);
			if (i != iDataLength - 1)
			{
				Append(' ');
			}
		}

		szString.Kill();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::AppendPointer(void* pv)
{
	Append("0x");
	return AppendHexHiLo(&pv, sizeof(void*));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChars::AppendFlag(uint32 msFlags, uint32 uiFlag, const char* szFlagName, bool bAppendComma)
{
	if (msFlags & uiFlag)
	{
		if (bAppendComma)
		{
			Append(", ");
		}
		Append(szFlagName);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChars::CountNewLines(void)
{
	return ::CountNewLines(mcText.GetData(), mcText.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CChars::Count(char c)
{
	size	i;
	char	cAt;
	size	iCount;
	size	uiLen;

	uiLen = Length();
	iCount = 0;
	for (i = 0; i < uiLen; i++)
	{
		cAt = mcText.GetValue(i);
		if (cAt == c)
		{
			iCount ++;
		}
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::LowerCase(void)
{
	ToLower(mcText.GetData());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::UpperCase(void)
{
	ToUpper(mcText.GetData());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::ReplaceNonTabsWithSpace(void)
{
	size	i;
	char	c;
	size	uiLen;

	uiLen = Length();
	for (i = 0; i < uiLen; i++)
	{
		c = mcText.GetValue(i);
		if (c != '\t')
		{
			mcText.SetValue(i, ' ');
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::PassifyNewlines(void)
{
	size	iNewLen;

	iNewLen = ::PassifyNewlines(mcText.GetData());
	if (iNewLen != SIZE_MAX)
	{
		mcText.Resize(iNewLen + 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CChars::FindLineContaining(char* szPosition, size* piLineNumber)
{
	size	i;
	size	iNumLineFeeds;
	size	uiLen;
	char*	szString;
	char*	szStartOfLine;
	char*	szPrevStartOfLine;
	char*	szCurrent;

	uiLen = Length();
	iNumLineFeeds = 0;
	szString = Text();
	szStartOfLine = Text();
	szPrevStartOfLine = szStartOfLine;
	for (i = 0; i < uiLen; i++)
	{
		szCurrent = &szString[i];
		if (*szCurrent == '\n')
		{
			szPrevStartOfLine = szStartOfLine;
			szStartOfLine = &szString[i + 1];
		}

		if (szCurrent == szPosition)
		{
			break;
		}

		if (*szCurrent == '\n')
		{
			iNumLineFeeds++;
		}
	}

	SafeAssign(piLineNumber, iNumLineFeeds);
	return szStartOfLine;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::MakeCPlusPlus(void)
{
	size	iIndex;
	bool	bStartOfLine;
	char	c;

	bStartOfLine = true;
	iIndex = 0;
	for (;;)
	{
		if (bStartOfLine)
		{
			Insert(iIndex, '"');
			iIndex++;
			bStartOfLine = false;
		}

		if (iIndex >= Length())
		{
			Insert(iIndex, '"');
			break;
		}
		c = GetChar(iIndex);
		if (c == '\n')
		{
			Insert(iIndex, "\\n\"");
			iIndex += 4;
			bStartOfLine = true;
			if (iIndex >= Length())
			{
				RemoveCharacter(iIndex - 1);
				break;
			}
		}
		else
		{
			iIndex++;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChars::WriteString(CFileWriter* pcWriter)
{
	CArrayChar c;

	if (!IsFakeEmpty())
	{
		return mcText.Write(pcWriter);
	}
	else
	{
		c.Fake(NULL, 0);
		return c.Write(pcWriter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChars::ReadString(CFileReader* pcReader)
{
	bool	bResult;

	bResult = mcText.Read(pcReader);
	if (bResult)
	{
		if ((mcText.NumElements() == 0) && (mcText.IsMalloc(&gcNullAllocator)))
		{
			InitEmpty();
		}
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChars::ReadChars(CFileReader* pcReader)
{
	uint32	iLength;

	if (!pcReader->ReadStringLength(&iLength))
	{
		return false;
	}

	if (iLength == 0)
	{
		Init();
		return true;
	}
	else if (iLength != 0)
	{
		Init('@', iLength - 1);
		if (!pcReader->ReadData(Text(), iLength)) 
		{ 
			return false; 
		}
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::CleanIfEmpty(void)
{
	if (mcText.NumElements() <= 1)
	{
		if (!IsFakeEmpty())
		{
			SetEmpty();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::InitEmpty(void)
{
	mcText.Fake(gszEmptyString, 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChars::IsFakeEmpty(void)
{
	return mcText.GetData() == gszEmptyString;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Unfake(void)
{
	mcText.Init();
	mcText.Add('\0');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::UnfakeIfFakeEmpty(void)
{
	if (IsFakeEmpty())
	{
		Unfake();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CChars::CopyIntoBuffer(char* szDest, size iDestLength)
{
	size		iLength;

	if (iDestLength != 0)
	{
		iLength = Length();
		if (iLength < iDestLength)
		{
			memcpy(szDest, Text(), iLength + 1);
		}
		else
		{
			memcpy(szDest, Text(), iDestLength - 2);
			szDest[iDestLength - 1] = '\0';
		}
		return szDest;
	}
	else if (iDestLength == 0)
	{
		return szDest;
	}
	else
	{
		strcpy(szDest, Text());
		return szDest;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Dump(void)
{
	CChars	szString;
	size	i;
	size	uiLen;

	uiLen = Length();
	if (uiLen <= 10000)
	{
		if (uiLen != 0)
		{
			EngineOutput(Text());
		}
		return;
	}

	for (i = 0; i < uiLen - 10000; i += 10000)
	{
		szString.Init(Text(), i, i + 10000);
		EngineOutput(szString.Text());
		szString.Kill();
	}

	szString.Init(Text(), i, uiLen);
	EngineOutput(szString.Text());
	szString.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Dump(const char* szString)
{
	CChars	c;

	c.Init(szString);
	c.Dump();
	c.Kill();
}

