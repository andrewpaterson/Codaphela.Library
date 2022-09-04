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
	//It does not make 'this' usable.  It will - rightly - null pointer.
	memset(this, 0, sizeof(CChars));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::InitLength(int iLength)
{
	if (iLength > 0)
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
CChars* CChars::Init(const char* sz)
{
	mcText.Init();
	if (!StrEmpty(sz))
	{
		Set(sz);
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
CChars* CChars::Init(CChars sz, int iStartInclusive, int iEndExclusive)
{
	if (iEndExclusive - iStartInclusive > 0)
	{
		mcText.Init();
		AppendSubString(sz, iStartInclusive, iEndExclusive);
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
CChars* CChars::Init(const char* sz, int iStartInclusive, int iEndExclusive)
{
	if (iEndExclusive - iStartInclusive > 0)
	{
		mcText.Init();
		AppendSubString(sz, iStartInclusive, iEndExclusive);
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
CChars* CChars::Init(const char* sz, int iStartInclusive)
{
	mcText.Init();
	Set(&sz[iStartInclusive]);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Init(CChars sz)
{
	if (!sz.Empty())
	{
		mcText.Init();
		mcText.Copy(&(sz.mcText));
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
CChars* CChars::Init(char cPadCharacter, int iNumber)
{
	if (iNumber > 0)
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
	int				iCount;

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
	int			iCount;

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
BOOL CChars::InitData2(const char* szData, int iDataLength)
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
void CChars::Fake(const char* sz)
{
	if (sz)
	{
		mcText.Fake((char*)sz, (int)strlen(sz) + 1);
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
void CChars::Fake(const char* sz, int iStartInclusive, int iEndExclusive)
{
	char* pcPosition;

	if (iEndExclusive - iStartInclusive > 0)
	{
		pcPosition = (char*)RemapSinglePointer(sz, iStartInclusive);
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
CChars* CChars::Reinit(void)
{
	Kill();
	return Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChars::Length(void)
{
	//Don't include the zero char at the end of the string.
	if (mcText.IsEmpty())
	{
		return 0;
	}
	return mcText.NumElements() - 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Set(CChars sz)
{
	int iLen;

	iLen = sz.Length()+1;
	if (iLen > 1)
	{
		SetNonNull(sz.Text(), iLen);
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
	int iLen;

	iLen = psz->Length() + 1;
	if (iLen > 1)
	{
		SetNonNull(psz->Text(), iLen);
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
void CChars::Set(const char* sz)
{
	size_t iLen;

	if (!StrEmpty(sz))
	{
		iLen = (strlen(sz)) + 1;
		SetNonNull(sz, iLen);
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
void CChars::SetNonNull(const char* sz, int iLen)
{
	if (!IsFakeEmpty())
	{
		mcText.Resize(iLen);
	}
	else
	{
		Unfake();
		mcText.Resize(iLen);
	}
	memcpy(mcText.GetData(), sz, iLen);
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
void CChars::SetChar(int iIndex, char c)
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
CChars* CChars::Append(const char* sz)
{
	int		iLen;
	char* pcPosition;

	if (!StrEmpty(sz))
	{
		iLen = (int)strlen(sz);
		pcPosition = PrivateGrow(iLen);
		memcpy(pcPosition, sz, iLen);
		pcPosition[iLen] = '\0';
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
CChars* CChars::Append(const char* sz, int iLen)
{
	char*	pcPosition;
	char*	pcZero;

	if (sz && iLen > 0)
	{
		pcPosition = PrivateGrow(iLen);
		memcpy(pcPosition, sz, iLen);
		pcZero = mcText.Tail();
		*pcZero = '\0';
	}

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Append(char c)
{
	char* pcReplace;

	UnfakeIfFakeEmpty();

	pcReplace = mcText.Tail();
	*pcReplace = c;
	mcText.Add(0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Append(CChars sz)
{
	Append(sz.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Append(CChars* psz)
{
	Append(psz->Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Append(int i)
{
	char sz[32];

	IntToString(sz, 32, i, 10);
	return Append(sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CChars::Append(int i, int iBase)
{
	char sz[32];

	IntToString(sz, 32, i, iBase);
	return Append(sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Append(unsigned long long int ulli)
{
	char sz[64];

	IntToString(sz, 64, ulli, 10);
	Append(sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Append(long long int lli)
{
	char sz[64];

	IntToString(sz, 64, lli, 10);
	Append(sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Append(unsigned int ui)
{
	char sz[32];

	IntToString(sz, 32, ui, 10);
	Append(sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Append(float f, int iMaxDecimals, BOOL bAppendF)
{
	char sz[32];

	FloatToString(sz, 32, f, iMaxDecimals, bAppendF);

	Append(sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Append(double d, int iMaxDecimals, BOOL bAppendD)
{
	char	sz[128];

	DoubleToString(sz, 128, d, iMaxDecimals, bAppendD);

	Append(sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::AppendQuoted(const char* sz)
{
	Append('"');
	Append(sz);
	Append('"');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::AppendQuoted(char c)
{
	Append('\'');
	Append(c);
	Append('\'');
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
CChars* CChars::AppendNewLine(CChars sz)
{
	Append(&sz);
	return AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char GetNumberAs1DigitHex(int nybble)
{
	if ((nybble >= 0) && (nybble <= 15))
	{
		return gszDigits[nybble];
	}
	return '?';
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::AppendHexHiLo(void* pv, int iNumBytes)
{
	int		i;
	char*	pb;
	int		nibble;
	char	c;

	for (i = iNumBytes-1; i >= 0; i--)
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::AppendHexLoHi(void* pv, int iNumBytes)
{
	int		i;
	char*	pb;
	int		nibble;
	char	c;

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

///////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::AppendBool(BOOL bValue)
{
	AppendBool(bValue, "True", "False");
}


/////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::AppendBool(BOOL bValue, const char* szTrue, const char* szFalse)
{
	if (bValue)
	{
		Append(szTrue);
	}
	else
	{
		Append(szFalse);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Insert(int iPos, char c)
{
	if ((iPos < 0) || (iPos >= mcText.NumElements()))
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
void CChars::Insert(int iPos, const char* szString)
{
	char*	pcNew;
	size_t	uiInsertLen;

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
void CChars::Insert(int iPos, CChars* pszString)
{
	char*	pcNew;
	size_t uiInsertLen;

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
char* CChars::Text(int iIndex)
{
	if (iIndex < Length())
	{
		return mcText.Get(iIndex);
	}
	else if (iIndex < 0)
	{
		return mcText.Get(0);
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
BOOL CChars::Empty(void)
{
	if (Length() == 0)
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::AppendList(const char* szFirst, ...)
{
	va_list			vaMarker;
	const char*		pc;
	int				iCount;

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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CChars::PrivateGrow(int iNumberOfCharacters)
{
	char* pszPosition;
	int		iPosition;

	if (mcText.IsEmpty())
	{
		if (iNumberOfCharacters > 0)
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
		if (IsFakeEmpty())
		{
			Unfake();
		}
		iPosition = mcText.AddNum(iNumberOfCharacters);
		pszPosition = (char*)RemapSinglePointer(mcText.GetData(), iPosition - 1);
		return pszPosition;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Append(char cPadCharacter, int iNumber)
{
	char*	pcPosition;

	if (iNumber > 0)
	{
		pcPosition = PrivateGrow(iNumber);
		memset(pcPosition, cPadCharacter, iNumber);
		pcPosition = mcText.Tail();
		*pcPosition = '\0';
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::LeftAlign(const char* sz, char cPadCharacter, unsigned int iWidth)
{
	size_t	uiLen;

	if (!StrEmpty(sz))
	{
		uiLen = strlen(sz);
		if (uiLen <= iWidth)
		{
			Append(sz);
			Append(cPadCharacter, iWidth - uiLen);
		}
		else
		{
			AppendSubString(sz, 0, iWidth);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::RightAlign(const char* sz, char cPadCharacter, unsigned int iWidth)
{
	size_t	uiLen;

	if (!StrEmpty(sz))
	{
		uiLen = strlen(sz);
		if (uiLen <= iWidth)
		{
			Append(cPadCharacter, iWidth - uiLen);
			Append(sz);
		}
		else
		{
			AppendSubString(sz, 0, iWidth);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::LeftAlign(CChars sz, char cPadCharacter, unsigned int iWidth)
{
	if (sz.Length() <= (int)iWidth)
	{
		Append(sz);
		Append(cPadCharacter, iWidth - sz.Length());
	}
	else
	{
		AppendSubString(sz, 0, iWidth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::RightAlign(CChars sz, char cPadCharacter, unsigned int iWidth)
{
	if (sz.Length() <= (int)iWidth)
	{
		Append(cPadCharacter, iWidth - sz.Length());
		Append(sz);
	}
	else
	{
		AppendSubString(sz, 0, iWidth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::RightAlign(char cPadCharacter, unsigned int iWidth)
{
	int		iLength;
	char*	pcDest;
	int		iOldWidth;

	if (Length() < (int)iWidth)
	{
		iLength = iWidth - Length();
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
void CChars::AppendSubString(const char* sz, int iLength)
{
	char*	pcPosition;

	if ((sz) && (iLength > 0))
	{
		pcPosition = PrivateGrow(iLength);
		memcpy(pcPosition, sz, iLength);
		(*mcText.Tail()) = '\0';
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::AppendSubString(CChars sz, int iStartInclusive, int iEndExclusive)
{
	const char* pcPosition;

	pcPosition = (char*)RemapSinglePointer((void*)sz.Text(), iStartInclusive);
	AppendSubString(pcPosition, iEndExclusive-iStartInclusive);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::AppendSubString(const char* sz, int iStartInclusive, int iEndExclusive)
{
	const char* pcPosition;

	pcPosition = (char*)RemapSinglePointer((void*)sz, iStartInclusive);
	AppendSubString(pcPosition, iEndExclusive-iStartInclusive);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::AppendSubString(const char* szStartInclusive, const char* szEndExclusive)
{
	AppendSubString(szStartInclusive, (int)(szEndExclusive-szStartInclusive));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::RemoveLastCharacter(void)
{
	char*	pcPosition;

	if (Length() > 0)
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
void CChars::RemoveFromStart(int iNumChars)
{
	Remove(0, iNumChars);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::RemoveFromEnd(int iNumChars)
{
	Remove(Length() - iNumChars, Length());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Remove(int iStartInclusive, int iEndExclusive)
{
	int	iLength;

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
void CChars::RemoveEnd(int iIndex)
{
	int	iToRemove;

	iToRemove = Length() - iIndex;
	if (iToRemove > 0)
	{
		if (iIndex > 0)
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
void CChars::RemoveCharacter(int iPos)
{
	if ((iPos < Length()) && (iPos >= 0))
	{
		mcText.RemoveAt(iPos, TRUE);
		CleanIfEmpty();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Split(CArrayChars* aszDest, char cSplitter)
{
	int		iPos;
	char*	pszPos;
	int		iMax;

	pszPos = Text();
	iMax = Length();
	for (;;)
	{
		iPos = FindFirstByte(pszPos, cSplitter, iMax);
		if (iPos != -1)
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
BOOL CChars::Equals(const char* szString)
{
	if (szString)
	{
		if (StringCompare(Text(), szString) == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChars::Equals(const char* szString, int iLen)
{
	if (iLen == Length())
	{
		if (memcmp(Text(), szString, iLen) == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChars::Equals(CChars szString)
{
	return Equals(szString.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChars::Equals(CChars* pszString)
{
	return Equals(pszString->Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChars::EqualsIgnoreCase(const char* szString)
{
	if (StringInsensitiveCompare(Text(), szString) == 0)
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChars::EqualsIgnoreCase(CChars szString)
{
	return EqualsIgnoreCase(szString.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChars::Contains(const char* szString)
{
	char*	pc;

	if (!szString) 
	{
		return FALSE;
	}

	pc = strstr(Text(), szString);
	if (pc)
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChars::ContainsIgnoreCase(const char* szString)
{
	char*	pc;

	pc = StrIStr(Text(), szString);
	if (pc)
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChars::Occurrences(const char* szString)
{
	int		iCount;
	int		iIndex;
	int		iFindLen;

	iCount = 0;
	iIndex = Find(0, szString);
	iFindLen = (int)strlen(szString);
	while (iIndex != -1)
	{
		iCount++;
		iIndex = Find(iIndex+iFindLen, szString);
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChars::StartsWith(const char* szString)
{
	if (szString == NULL)
	{
		return FALSE;
	}

	return SubStringEquals(0, szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChars::StartsWithIgnoreCase(const char* szString)
{
	if (szString == NULL)
	{
		return FALSE;
	}

	return SubStringEqualsIgnoreCase(0, szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChars::EndsWith(const char* szString)
{
	if (szString == NULL)
	{
		return FALSE;
	}

	return SubStringEquals(Length() - (int)strlen(szString), szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChars::EndsWith(char c)
{
	int		iLength;

	iLength = Length();
	if (iLength > 0)
	{
		if (Text()[iLength - 1] == c)
		{
			return TRUE;
		}
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChars::EndsWithIgnoreCase(const char* szString)
{
	if (szString == NULL)
	{
		return FALSE;
	}

	return SubStringEqualsIgnoreCase(Length() - (int)strlen(szString), szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChars::SubStringEquals(int iStart, const char* szString)
{
	int	i;
	int	j;
	int	iLen;
	int iSubLen;

	if (iStart < 0)
	{
		return FALSE;
	}

	iLen = Length();
	if (iLen == 0)
	{
		iSubLen = strlen(szString);
		if (iSubLen != 0)
		{
			return FALSE;
		}
		else
		{
			return iStart == 0;
		}
	}

	if (iStart >= iLen)
	{
		return FALSE;
	}

	for (i = iStart, j = 0; i < iLen; i++, j++)
	{
		if (szString[j] != 0)
		{
			if (mcText.GetValue(i) != szString[j])
			{
				return FALSE;
			}
		}
		else
		{
			break;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChars::SubStringEqualsIgnoreCase(int iStart, const char* szString)
{
	int	i;
	int	j;
	int	iLen;
	int             c1;
	int             c2;

	if (iStart < 0)
	{
		return FALSE;
	}

	iLen = Length();
	for (i = iStart, j = 0; i < iLen; i++, j++)
	{
		if (szString[j] != 0)
		{
			c1 = ToLower(mcText.GetValue(i));
			c2 = ToLower(szString[j]);
			if (c1 != c2)
			{
				return FALSE;
			}
		}
		else
		{
			break;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChars::FindFromEnd(const char* szString)
{
	int iOtherLen;

	iOtherLen = (int)strlen(szString);
	return FindFromEnd(Length() - iOtherLen, szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChars::FindFromEnd(int iIndex, const char* szString)
{
	int	i;

	if ((iIndex < 0) || (iIndex > Length()))
	{
		return -1;
	}

	for (i = iIndex; i >= 0; i--)
	{
		if (SubStringEquals(i, szString))
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChars::FindFromEnd(char c)
{
	return FindFromEnd(Length() - 1, c);

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChars::FindFromEnd(int iIndex, char c)
{
	int	i;

	for (i = iIndex; i >= 0; i--)
	{
		if (mcText.GetValue(i) == c)
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChars::Find(const char* szString)
{
	return Find(0, szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChars::Find(int iIndex, const char* szString)
{
	int iOtherLen;
	int	i;

	iOtherLen = (int)strlen(szString);

	for (i = iIndex; i <= Length() - iOtherLen; i++)
	{
		if (SubStringEquals(i, szString))
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChars::Find(int iIndex, char c)
{
	int	i;

	if (iIndex < 0)
	{
		iIndex = 0;
	}

	for (i = iIndex; i < Length(); i++)
	{
		if (mcText.GetValue(i) == c)
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChars::Find(char c)
{
	return Find(0, c);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChars::FindDigit(int iIndex)
{
	int		i;
	char	c;

	if (iIndex < 0)
	{
		iIndex = 0;
	}

	for (i = iIndex; i < Length(); i++)
	{
		c = *mcText.Get(i);
		if ((c >= '0') && (c <= '9'))
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CChars::GetChar(int iIndex)
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
	int		i;
	int		j;
	CChars	cTemp;
	char	cNew;
	char	cOld;

	//This will find the index of each character in this string in the old string.

	paiOldToNewIndices->SetUsedElements(szOldString.Length());
	paiOldToNewIndices->SetArrayValues(-1);

	paiNewToOldIndices->SetUsedElements(Length());
	paiNewToOldIndices->SetArrayValues(-1);

	cTemp.Init(szOldString);
	for (j = 0; j < Length(); j++)
	{
		cNew = GetChar(j);
		for (i = 0; i < cTemp.Length(); i++)
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
BOOL CChars::IsWhiteSpace(int iPos, BOOL bIncludeNewLines)
{
	char	c;

	c = GetChar(iPos);
	return ::IsWhiteSpace(c, bIncludeNewLines);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChars::IsWhiteSpace(void)
{
	int		i;
	int		iLen;

	iLen = Length();
	for (i = 0; i < iLen; i++)
	{
		if (!IsWhiteSpace(i, TRUE))
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::SetLength(int iLength)
{
	if (iLength > 0)
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
int CChars::FindEndOfLeadingWhiteSpace(BOOL bIncludeNewLines)
{
	int		iLength;
	int		iStart;

	iLength = Length();
	for (iStart = 0; iStart < iLength; iStart++)
	{
		if (!IsWhiteSpace(iStart, bIncludeNewLines))
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
int CChars::FindStartOfTrailingWhiteSpace(BOOL bIncludeNewLines)
{
	int		iLength;
	int		iEnd;

	iLength = Length();
	for (iEnd = iLength-1; iEnd >= 0; iEnd--)
	{
		if (!IsWhiteSpace(iEnd, bIncludeNewLines))
		{
			break;
		}
	}
	return iEnd;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChars::StripWhiteSpace(BOOL bIncludeNewLines)
{
	int		iLength;
	int		iStart;
	int		iEnd;

	iStart = FindEndOfLeadingWhiteSpace(bIncludeNewLines);
	iEnd = FindStartOfTrailingWhiteSpace(bIncludeNewLines);

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
int CChars::Replace(char cFind, char cReplace)
{
	int		i;
	int		iLength;
	int		iCount;

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
int CChars::Replace(const char* szFind, const char* szReplace)
{
	int		iReplaceLen;
	int		iFindLen;
	int		iDifference;

	if (szFind == NULL)
	{
		return 0;
	}

	iFindLen = (int)strlen(szFind);
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
		iReplaceLen = (int)strlen(szReplace);
	}

	iDifference = iReplaceLen-iFindLen;

	if (iDifference > 0)
	{
		return ReplaceWithLonger(szFind, szReplace, iFindLen, iDifference);
	}
	else if (iDifference < 0)
	{
		return ReplaceWithShorter(szFind, szReplace, iReplaceLen, iFindLen, iDifference);
	}
	else
	{
		return ReplaceWithEqualLength(szFind, szReplace, iFindLen, iDifference);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChars::ReplaceWithEqualLength(const char* szFind, const char* szReplace, int iFindLen, int iDifference)
{
	int		iCount;
	int		iIndex;

	iCount = 0;
	iIndex = Find(0, szFind);
	while (iIndex != -1)
	{
		Overwrite(iIndex, szReplace);
		iCount++;
		iIndex = Find(iIndex+iFindLen, szFind);
	}
	return iCount;}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChars::ReplaceWithShorter(const char* szFind, const char* szReplace, int iReplaceLen, int iFindLen, int iDifference)
{
	int		iCount;
	int		iIndex;
	int		iTotalDifference;
	int		iOldIndex;
	int		iDestPos;
	int		iSize;
	int		iEnd;

	//Remember that iDifference is NEGATIVE!
	iCount = 0;
	iIndex = Find(0, szFind);
	if (iIndex == -1)
	{
		return 0;
	}

	iOldIndex = 0;
	for (;;)
	{
		iSize = iIndex - iOldIndex + iDifference;

		if (iReplaceLen != 0)
		{
			if (iCount != 0)
			{
				memcpy(mcText.Get(iDestPos), mcText.Get(iOldIndex + iFindLen), iSize - 1);
			}
			Overwrite(iIndex + (iCount * iDifference), szReplace);
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
		iDestPos = iOldIndex + iReplaceLen + (iDifference * (iCount-1));
		iIndex = Find(iIndex + iFindLen, szFind);
		if (iIndex == -1)
		{
			iTotalDifference = iDifference * iCount;
			iEnd = Length() + iTotalDifference;
			iSize = Length() - iOldIndex + iDifference;
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
	mcText.AddNum(iTotalDifference);
	(*mcText.Tail()) = '\0';
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChars::ReplaceWithLonger(const char* szFind, const char* szReplace, int iFindLen, int iDifference)
{
	int		iCount;
	int		iIndex;
	int		i;
	int		iTotalDifference;
	int		iOldIndex;

	iCount = Occurrences(szFind);
	if (iCount == 0)
	{
		return 0;
	}

	iTotalDifference = iDifference * iCount;
	mcText.AddNum(iTotalDifference);
	(*mcText.Tail()) = '\0';

	iIndex = Length() - (iFindLen + iTotalDifference);
	for (i = iCount-1; i >= 0; i--)
	{
		iOldIndex = iIndex;
		iIndex = FindFromEnd(iIndex, szFind);
		memcpy(mcText.Get(iIndex + iFindLen + (iDifference * (i+1))), mcText.Get(iIndex + iFindLen), iOldIndex - iIndex);
		Overwrite(iIndex + (iDifference * i), szReplace);
		iIndex -= iFindLen;
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Overwrite(int iIndex, const char* szReplace)
{
	int		i;
	int		j;
	int		iLen;

	if (StrEmpty(szReplace))
	{
		return;
	}

	iLen = Length();
	for (i = iIndex, j = 0; i < iLen; i++, j++)
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
CChars* CChars::AppendData(const char* szData, size_t iMaxLength)
{
	size_t		i;
	size_t		iLength;

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
CChars* CChars::AppendData(const char* szData, size_t iDataLength, size_t iMaxLength)
{
	int				i;
	int				iLength;
	unsigned char	c;
	BOOL			bLastReadable;
	
	if (iDataLength > iMaxLength)
	{
		iLength = iMaxLength;
	}
	else
	{
		iLength = iDataLength;
	}

	bLastReadable = TRUE;
	for (i = 0; i < iLength; i++)
	{
		if (!bLastReadable)
		{
			Append(",");
		}

		c = szData[i];
		if (c == 0)
		{
			if ((bLastReadable) && (i > 0))
			{
				Append("\\0");
			}
			else
			{
				Append(" 0x00");
			}
			bLastReadable = FALSE;
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
			bLastReadable = FALSE;
		}
		else if ((c >= 32) && (c <= 126))
		{
			if (!bLastReadable)
			{
				Append(' ');
			}
			bLastReadable = TRUE;
			Append((char)c);
		}
		else
		{
			bLastReadable = FALSE;
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
BOOL CChars::AppendData2(const char* szData, size_t iDataLength)
{
	size_t			i;
	int				iPrintable;
	unsigned char	c;
	float			fPrintable;
	CChars			sz;

	if ((szData == NULL) || (iDataLength == 0))
	{
		return FALSE;
	}

	iPrintable = StrPrintable(szData, iDataLength);
	fPrintable = (float)iPrintable / (float)iDataLength;

	if (fPrintable >= 0.9f)
	{
		Append(szData, iDataLength);
		return FALSE;
	}
	else
	{
		Append("0x");

		sz.Init();

		for (i = 0; i < iDataLength; i++)
		{
			c = szData[i];

			sz.Clear();
			sz.Append((int)c, 16);
			sz.RightAlign('0', 2);

			Append(sz);
			if (i != iDataLength - 1)
			{
				Append(' ');
			}
		}

		sz.Kill();
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::AppendPointer(void* pv)
{
	Append("0x");
	AppendHexHiLo(&pv, sizeof(void*));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChars::AppendFlag(unsigned int msFlags, unsigned int uiFlag, const char* szFlagName, BOOL bAppendComma)
{
	if (msFlags & uiFlag)
	{
		if (bAppendComma)
		{
			Append(", ");
		}
		Append(szFlagName);
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChars::CountNewLines(void)
{
	return ::CountNewLines(mcText.GetData(), mcText.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChars::Count(char c)
{
	int		i;
	char	cAt;
	int		iCount;

	iCount = 0;
	for (i = 0; i < Length(); i++)
	{
		cAt = GetChar(i);
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
void CChars::PassifyNewlines(void)
{
	int	iNewLen;

	iNewLen = ::PassifyNewlines(mcText.GetData());
	if (iNewLen != -1)
	{
		mcText.Resize(iNewLen+1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CChars::FindLineContaining(char* szPosition, int* piLineNumber)
{
	int		i;
	int		iNumLineFeeds;
	int		iLen;
	char*	szString;
	char*	szStartOfLine;
	char*	szPrevStartOfLine;
	char*	szCurrent;

	iLen = Length();
	iNumLineFeeds = 0;
	szString = Text();
	szStartOfLine = Text();
	szPrevStartOfLine = szStartOfLine;
	for (i = 0; i < iLen; i++)
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
	int		iIndex;
	BOOL	bStartOfLine;
	char	c;

	bStartOfLine = TRUE;
	iIndex = 0;
	for (;;)
	{
		if (bStartOfLine)
		{
			Insert(iIndex, '"');
			iIndex++;
			bStartOfLine = FALSE;
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
			bStartOfLine = TRUE;
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
BOOL CChars::WriteString(CFileWriter* pcWriter)
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
BOOL CChars::ReadString(CFileReader* pcReader)
{
	BOOL bResult;

	bResult = mcText.Read(pcReader);
	if (bResult)
	{
		if ((mcText.NumElements() == 0) && (mcText.IsMalloc(&gcNullAllocator)))
		{
			InitEmpty();
		}
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChars::ReadChars(CFileReader* pcReader)
{
	int	iLength;

	if (!pcReader->ReadStringLength(&iLength))
	{
		return FALSE;
	}

	if (iLength == 0)
	{
		Init();
		return TRUE;
	}
	else if (iLength > 0)
	{
		Init('@', iLength-1);
		if (!pcReader->ReadData(Text(), iLength)) 
		{ 
			return FALSE; 
		}
		return TRUE;
	}
	else
	{
		return FALSE;
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
BOOL CChars::IsFakeEmpty(void)
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
char* CChars::CopyIntoBuffer(char* szDest, int iDestLength)
{
	int		iLength;

	if (iDestLength > 0)
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
	CChars	sz;
	int		i;

	if (Length() <= 10000)
	{
		if (Length() > 0)
		{
			EngineOutput(Text());
		}
		return;
	}

	for (i = 0; i < Length()-10000; i+=10000)
	{
		sz.Init(Text(), i, i+10000);
		EngineOutput(sz.Text());
		sz.Kill();
	}
	sz.Init(Text(), i, Length());
	EngineOutput(sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChars::Dump(const char* sz)
{
	CChars	c;

	c.Init(sz);
	c.Dump();
	c.Kill();
}

