/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
//No include guard: CharsCode.inl is included once per string size (ArraySizeNatural.inl and
//ArraySizeLarge.inl define the names and types it is written with).

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHARS::_Init(void)
{
	//This makes it safe to call kill.
	//It does not make 'this' usable.  It will - rightly - NULL pointer.
	mcText._Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::InitLength(ASIZE iLength)
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
CHARS* CHARS::InitLength(CMallocator* pcMalloc, ASIZE iLength)
{
	if (iLength != 0)
	{
		mcText.Init(pcMalloc);
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
CHARS* CHARS::Init(void)
{
	InitEmpty();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::Init(const char* szString)
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
CHARS* CHARS::Init(CHARS szString, ASIZE iStartInclusive, ASIZE iEndExclusive)
{
	if ((iEndExclusive > iStartInclusive) && (iEndExclusive != ARRAY_SIZE_MAX))
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
CHARS* CHARS::Init(const char* szString, ASIZE iStartInclusive, ASIZE iEndExclusive)
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
CHARS* CHARS::Init(const char* szString, ASIZE iStartInclusive)
{
	mcText.Init();
	Set(&szString[iStartInclusive]);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::Init(CHARS szString)
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
CHARS* CHARS::Init(CHARS* psz)
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
CHARS* CHARS::Init(char cPadCharacter, ASIZE iNumber)
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
CHARS* CHARS::Init(CExternalString* pcString)
{
	return Init(pcString->msz, 0, pcString->miLen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::InitList(const char* szFirst, ...)
{
	InitEmpty();

	va_list			vaMarker;
	const char*		pc;
	ASIZE			iCount;

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
CHARS* CHARS::InitList(CHARS* szFirst, ...)
{
	InitEmpty();

	va_list		vaMarker;
	CHARS*		pc;
	ASIZE		iCount;

	if (szFirst)
	{
		iCount = 0;
		pc = szFirst;

		va_start(vaMarker, szFirst);
		while (pc && pc->Length() != 0)
		{
			Append(pc->Text());
			iCount++;
			pc = va_arg(vaMarker, CHARS*);
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
bool CHARS::InitData2(const char* szData, ASIZE iDataLength)
{
	InitEmpty();

	return AppendData2(szData, iDataLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHARS::Kill(void)
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
void CHARS::DumpKill(void)
{
	Dump();
	Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHARS::Fake(const char* szString)
{
	if (szString)
	{
		mcText.Fake((char*)szString, CStringLength(szString) + 1);
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
void CHARS::Fake(const char* szString, ASIZE iStartInclusive, ASIZE iEndExclusive)
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
CHARS* CHARS::ReInit(void)
{
	Kill();
	return Init();
}


//////////////////////////////////////////////////////////////////////////
//
// szString's length if it fits (ARRAY_SIZE_MAX - 1 characters and the zero).
// A longer one is an overflow (ARRAY_CHECK_SIZE): reported in debug, and
// treated as empty rather than cut short.
//
//////////////////////////////////////////////////////////////////////////
ASIZE CHARS::CStringLength(const char* szString)
{
	lsize	uiLength;

	uiLength = strlen(szString);
#ifdef ARRAY_CHECK_SIZE
	if (uiLength >= ARRAY_SIZE_MAX)
	{
#ifdef _DEBUG
		gcUserError.Set("A string counted with size can't hold a C string of MAX_SIZE characters or more.  Use CLargeChars.");
#endif // _DEBUG
		return 0;
	}
#endif // ARRAY_CHECK_SIZE
	return (ASIZE)uiLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ASIZE CHARS::Length(void)
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
void CHARS::Set(CHARS szString)
{
	ASIZE uiLen;

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
void CHARS::Set(CHARS* psz)
{
	ASIZE uiLen;

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
void CHARS::Set(const char* szString)
{
	ASIZE uiLen;

	if (!StrEmpty(szString))
	{
		uiLen = (CStringLength(szString)) + 1;
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
void CHARS::SetNonNull(const char* szString, ASIZE uiLen)
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
	ARRAY_MEMCPY(mcText.GetData(), szString, uiLen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHARS::SetEmpty(void)
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
void CHARS::SetChar(ASIZE iIndex, char c)
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
CHARS* CHARS::Append(const char* szString)
{
	ASIZE	uiLen;
	char*	pcPosition;

	if (!StrEmpty(szString))
	{
		uiLen = CStringLength(szString);
		pcPosition = PrivateGrow(uiLen);
		ARRAY_MEMCPY(pcPosition, szString, uiLen);
		pcPosition[uiLen] = '\0';
	}

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::Append(CExternalString* pcString)
{
	char*	pcPosition;

	if (pcString && pcString->miLen > 0)
	{
		pcPosition = PrivateGrow(pcString->miLen);
		ARRAY_MEMCPY(pcPosition, pcString->msz, pcString->miLen);
		pcPosition[pcString->miLen] = '\0';
	}

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::Append(const char* szString, ASIZE uiLen)
{
	char*	pcPosition;
	char*	pcZero;

	if (szString && uiLen > 0)
	{
		pcPosition = PrivateGrow(uiLen);
		ARRAY_MEMCPY(pcPosition, szString, uiLen);
		pcZero = mcText.Tail();
		*pcZero = '\0';
	}

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::Append(char cPadCharacter)
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
CHARS* CHARS::Append(CHARS szString)
{
	Append(szString.Text());
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::Append(CHARS* psz)
{
	Append(psz->Text());
	return this;
}


#ifdef CHARS_NATURAL
//////////////////////////////////////////////////////////////////////////
//
// A large string appends a natural one.
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::Append(CHARS_NATURAL szString)
{
	Append(szString.Text());
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::Append(CHARS_NATURAL* psz)
{
	Append(psz->Text());
	return this;
}
#endif // CHARS_NATURAL


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::Append(CCharsImmutable* psz)
{
	Append(psz->Text());
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::Append(int32 i)
{
	char szString[32];

	IntToString(szString, 32, i);
	return Append(szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::Append(int32 i, uint16 iBase)
{
	char szString[32];

	IntToString(szString, 32, i, iBase);
	return Append(szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::Append(uint64 ulli)
{
	char szString[64];

	LongToString(szString, 64, ulli);
	return Append(szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::Append(int64 lli)
{
	char szString[64];

	LongToString(szString, 64, lli);
	return Append(szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::Append(uint32 ui)
{
	char szString[32];

	IntToString(szString, 32, ui);
	return Append(szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::Append(uint32 ui, uint16 iBase)
{
	char szString[32];

	IntToString(szString, 32, ui, iBase);
	return Append(szString);
}


#ifdef __W65C816__
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::Append(int16 i)
{
	return Append((int32)i);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::Append(int16 i, uint16 iBase)
{
	return Append((int32)i, iBase);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::Append(uint16 ui)
{
	return Append((uint32)ui);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::Append(uint16 ui, uint16 iBase)
{
	return Append((uint32)ui, iBase);
}
#endif // __W65C816__


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::Append(float f, ASIZE iMaxDecimals, bool bAppendF)
{
	char szString[32];

	if (!FloatToString(szString, 32, f, (int)iMaxDecimals, bAppendF))
	{
		return this;
	}
	return  Append(szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::Append(double d, ASIZE iMaxDecimals, bool bAppendD)
{
	char	szString[128];

	if (!DoubleToString(szString, 128, d, (int)iMaxDecimals, bAppendD))
	{
		return this;
	}
	return Append(szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::AppendQuoted(const char* szString)
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
CHARS* CHARS::AppendQuoted(char c)
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
CHARS* CHARS::AppendNewLine(void)
{
	return Append("\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::AppendCarriageReturn(void)
{
	return Append("\r");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::AppendNewLine(CHARS szString)
{
	Append(&szString);
	return AppendNewLine();
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::AppendHexHiLo(void* pv, ASIZE iNumBytes)
{
	ASIZE	i;
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
CHARS* CHARS::AppendHexLoHi(void* pv, ASIZE iNumBytes)
{
	ASIZE	i;
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
CHARS* CHARS::AppendBool(bool bValue)
{
	return AppendBool(bValue, "True", "False");
}


/////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::AppendBool(bool bValue, const char* szTrue, const char* szFalse)
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
void CHARS::Insert(ASIZE iPos, char c)
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
void CHARS::Insert(ASIZE iPos, char c, ASIZE uiCount)
{
	ASIZE i;

	mcText.InsertNumAt(uiCount, iPos);
	for (i = 0; i < uiCount; i++)
	{
		mcText.Set(iPos + i, &c);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHARS::Insert(ASIZE iPos, const char* szString)
{
	char*	pcNew;
	ASIZE	uiInsertLen;

	if (StrEmpty(szString))
	{
		return;
	}

	UnfakeIfFakeEmpty();
	uiInsertLen = CStringLength(szString);
	pcNew = mcText.InsertNumAt(uiInsertLen, iPos);
	if (pcNew)
	{
		ARRAY_MEMCPY(pcNew, szString, uiInsertLen);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHARS::Insert(ASIZE iPos, CHARS* pszString)
{
	char*	pcNew;
	ASIZE uiInsertLen;

	if (pszString == NULL || pszString->Empty())
	{
		return;
	}

	UnfakeIfFakeEmpty();
	uiInsertLen = pszString->Length();
	pcNew = mcText.InsertNumAt(uiInsertLen, iPos);
	if (pcNew)
	{
		ARRAY_MEMCPY(pcNew, pszString->Text(), uiInsertLen);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHARS::Minimize(void)
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
char* CHARS::Text(void)
{
	return mcText.GetData();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CHARS::Text(ASIZE iIndex)
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
bool CHARS::Empty(void)
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
CHARS* CHARS::AppendList(const char* szFirst, ...)
{
	va_list			vaMarker;
	const char*		pc;
	ASIZE				iCount;

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
char* CHARS::PrivateGrow(ASIZE iNumberOfCharacters)
{
	char*	pszPosition;
	ASIZE	iPosition;

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
CHARS* CHARS::Append(char cPadCharacter, ASIZE iNumber)
{
	char*	pcPosition;

	if ((iNumber != 0) && (iNumber != ARRAY_NOT_FOUND))
	{
		pcPosition = PrivateGrow(iNumber);
		ARRAY_MEMSET(pcPosition, cPadCharacter, iNumber);
		pcPosition = mcText.Tail();
		*pcPosition = '\0';
	}

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHARS::LeftAlign(const char* szString, char cPadCharacter, ASIZE uiWidth)
{
	ASIZE	uiLen;

	uiLen = CStringLength(szString);
	if (uiLen != 0)
	{
		uiLen = CStringLength(szString);
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
void CHARS::RightAlign(const char* szString, char cPadCharacter, ASIZE uiWidth)
{
	ASIZE	uiLen;

	uiLen = CStringLength(szString);
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
void CHARS::LeftAlign(CHARS szString, char cPadCharacter, ASIZE uiWidth)
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
void CHARS::RightAlign(CHARS szString, char cPadCharacter, ASIZE uiWidth)
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
void CHARS::RightAlign(ASIZE uiOffset, char cPadCharacter, ASIZE uiWidth)
{
	ASIZE	uiLen;

	uiLen = Length() - uiOffset;
	if (uiLen != 0)
	{
		if (uiLen <= uiWidth)
		{
			Insert(uiOffset, cPadCharacter, uiWidth - uiLen);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHARS::RightAlign(char cPadCharacter, ASIZE uiWidth)
{
	ASIZE	iLength;
	char*	pcDest;
	ASIZE	iOldWidth;

	if (Length() < uiWidth)
	{
		iLength = uiWidth - Length();
		iOldWidth = Length();
		PrivateGrow(iLength);
		pcDest = (char*)RemapSinglePointer(mcText.GetData(), iLength);
		ARRAY_MEMMOVE(pcDest, mcText.GetData(), iOldWidth);
		ARRAY_MEMSET(mcText.GetData(), cPadCharacter, iLength);
		(*mcText.Tail()) = '\0';
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::AppendSubString(const char* szString, ASIZE iLength)
{
	char*	pcPosition;

	if ((szString) && (iLength != 0))
	{
		pcPosition = PrivateGrow(iLength);
		ARRAY_MEMCPY(pcPosition, szString, iLength);
		(*mcText.Tail()) = '\0';
	}

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::AppendSubString(CHARS szString, ASIZE iStartInclusive, ASIZE iEndExclusive)
{
	const char* pcPosition;

	pcPosition = (char*)RemapSinglePointer((void*)szString.Text(), iStartInclusive);
	return AppendSubString(pcPosition, iEndExclusive-iStartInclusive);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::AppendSubString(CHARS* pszString, ASIZE iStartInclusive, ASIZE iEndExclusive)
{
	const char* pcPosition;

	pcPosition = (char*)RemapSinglePointer((void*)pszString->Text(), iStartInclusive);
	return AppendSubString(pcPosition, iEndExclusive-iStartInclusive);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::AppendSubString(const char* szString, ASIZE iStartInclusive, ASIZE iEndExclusive)
{
	const char* pcPosition;

	pcPosition = (char*)RemapSinglePointer((void*)szString, iStartInclusive);
	return AppendSubString(pcPosition, iEndExclusive-iStartInclusive);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHARS* CHARS::AppendSubString(const char* szStartInclusive, const char* szEndExclusive)
{
	return AppendSubString(szStartInclusive, (ASIZE)(szEndExclusive-szStartInclusive));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHARS::RemoveLastCharacter(void)
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
void CHARS::RemoveFromStart(ASIZE iNumChars)
{
	Remove(0, iNumChars);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHARS::RemoveFromEnd(ASIZE iNumChars)
{
	ASIZE iLength;

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
void CHARS::Remove(ASIZE iStartInclusive, ASIZE iEndExclusive)
{
	ASIZE	iLength;

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
void CHARS::RemoveEnd(ASIZE iIndex)
{
	ASIZE	iToRemove;
	ASIZE	uiLength;

	uiLength = Length();
	if (uiLength >= iIndex)
	{
		iToRemove = uiLength - iIndex;
		if (iToRemove != 0)
		{
			if (iIndex != 0)
			{
				mcText.RemoveRange(iIndex, uiLength);
			}
			else
			{
				SetEmpty();
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHARS::RemoveCharacter(ASIZE iPos)
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
void CHARS::Split(CArrayChars* aszDest, char cSplitter)
{
	lsize	iPos;
	char*	pszPos;
	ASIZE	iMax;

	pszPos = Text();
	iMax = Length();
	for (;;)
	{
		//FindFirstByte counts with lsize whatever the size type.  Each part is a CChars.
		iPos = FindFirstByte((uint8*)pszPos, cSplitter, iMax);
		if (iPos != ARRAY_LARGE_ELEMENT_NOT_FOUND)
		{
			aszDest->Add(pszPos, 0, (size)iPos);
			iPos++;
			pszPos = (char*)RemapSinglePointer((void*)pszPos, iPos);
			iMax -= (ASIZE)iPos;
		}
		else
		{
			aszDest->Add(pszPos, 0, (size)iMax);
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHARS::SplitLines(CArrayChars* aszDest)
{
	//This assumes new lines are passified.
	Split(aszDest, '\n');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHARS::Equals(const char* szString)
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
bool CHARS::Equals(const char* szString, ASIZE uiLen)
{
	if (uiLen == Length())
	{
		if (ARRAY_MEMCMP(Text(), szString, uiLen) == 0)
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
bool CHARS::Equals(CHARS szString)
{
	return Equals(szString.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHARS::Equals(CHARS* pszString)
{
	return Equals(pszString->Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHARS::EqualsIgnoreCase(const char* szString)
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
bool CHARS::EqualsIgnoreCase(CHARS szString)
{
	return EqualsIgnoreCase(szString.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHARS::Contains(const char* szString)
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
bool CHARS::ContainsIgnoreCase(const char* szString)
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
bool CHARS::Contains(char c)
{
	return Find(0, c) != ARRAY_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ASIZE CHARS::Occurrences(const char* szString)
{
	ASIZE	iCount;
	ASIZE	iIndex;
	ASIZE	iFindLen;

	iFindLen = CStringLength(szString);
	if (iFindLen == 0)
	{
		return 0;
	}

	iIndex = Find(0, szString);
	if (iIndex == ARRAY_NOT_FOUND)
	{
		return 0;
	}

	iCount = 0;
	for (;;)
	{
		iCount++;
		iIndex += iFindLen;
		iIndex = Find(iIndex, szString);
		if (iIndex == ARRAY_NOT_FOUND)
		{
			return iCount;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHARS::StartsWith(const char* szString)
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
bool CHARS::StartsWithIgnoreCase(const char* szString)
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
bool CHARS::StartsWith(char c)
{
	ASIZE		iLength;

	iLength = Length();
	if (iLength != 0)
	{
		if (Text()[0] == c)
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
bool CHARS::EndsWith(const char* szString)
{
	ASIZE	uiLen;

	if (szString == NULL)
	{
		return false;
	}

	uiLen = CStringLength(szString);
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
bool CHARS::EndsWith(char c)
{
	ASIZE		iLength;

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
bool CHARS::EndsWithIgnoreCase(const char* szString)
{
	ASIZE	uiLen;

	if (szString == NULL)
	{
		return false;
	}

	uiLen = CStringLength(szString);
	if (uiLen > Length())
	{
		return false;
	}

	return SubStringEqualsIgnoreCase(Length() - CStringLength(szString), szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHARS::SubStringEquals(ASIZE iStart, const char* szString)
{
	ASIZE	i;
	ASIZE	j;
	ASIZE	uiLen;
	ASIZE	iSubLen;
	char*	szText;

	szText = Text();
	uiLen = Length();
	if (uiLen == 0)
	{
		iSubLen = CStringLength(szString);
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
			if (szText[i] != szString[j])
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
bool CHARS::SubStringEqualsIgnoreCase(ASIZE iStart, const char* szString)
{
	ASIZE	i;
	ASIZE	j;
	ASIZE	uiLen;
	ASIZE    c1;
	ASIZE    c2;
	char*	szText;

	szText = Text();
	uiLen = Length();
	for (i = iStart, j = 0; i < uiLen; i++, j++)
	{
		if (szString[j] != 0)
		{
			c1 = ToLower(szText[i]);
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
ASIZE CHARS::FindFromEnd(const char* szString)
{
	ASIZE iOtherLen;

	iOtherLen = CStringLength(szString);
	return FindFromEnd(Length() - iOtherLen, szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ASIZE CHARS::FindFromEnd(ASIZE iIndex, const char* szString)
{
	ASIZE	i;

	if (iIndex > Length())
	{
		return ARRAY_NOT_FOUND;
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
	return ARRAY_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ASIZE CHARS::FindFromEnd(char c)
{
	if (mcText.IsEmpty() || IsFakeEmpty())
	{
		return ARRAY_NOT_FOUND;
	}
	
	return FindFromEnd(Length() - 1, c);

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ASIZE CHARS::FindFromEnd(ASIZE iIndex, char c)
{
	ASIZE	i;
	char*	szText;
	ASIZE	uiLength;

	//An index past the end starts at the last character.
	uiLength = Length();
	if (uiLength == 0)
	{
		return ARRAY_NOT_FOUND;
	}
	if (iIndex >= uiLength)
	{
		iIndex = uiLength - 1;
	}

	szText = Text();
	i = iIndex + 1;
	do
	{
		i--;
		if (szText[i] == c)
		{
			return i;
		}
	}
	while (i != 0);
	return ARRAY_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ASIZE CHARS::Find(const char* szString)
{
	return Find(0, szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ASIZE CHARS::Find(ASIZE iIndex, const char* szString)
{
	ASIZE	iOtherLen;
	ASIZE	i;
	ASIZE	uiLen;
	ASIZE	iDiff;

	uiLen = Length();
	if (iIndex > uiLen)
	{
		return ARRAY_NOT_FOUND;
	}

	iOtherLen = CStringLength(szString);
	if (iOtherLen > uiLen - iIndex)
	{
		return ARRAY_NOT_FOUND;
	}

	iDiff = uiLen - iOtherLen;
	for (i = iIndex; i <= iDiff; i++)
	{
		if (SubStringEquals(i, szString))
		{
			return i;
		}
	}
	return ARRAY_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ASIZE CHARS::Find(ASIZE iIndex, char c)
{
	ASIZE	i;
	ASIZE	uiLen;
	char*	szText;

	szText = Text();
	uiLen = Length();
	for (i = iIndex; i < uiLen; i++)
	{
		if (szText[i] == c)
		{
			return i;
		}
	}
	return ARRAY_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ASIZE CHARS::Find(char c)
{
	return Find(0, c);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ASIZE CHARS::FindDigit(ASIZE iIndex)
{
	ASIZE	i;
	char	c;
	ASIZE	uiLen;

	uiLen = Length();
	for (i = iIndex; i < uiLen; i++)
	{
		c = *mcText.Get(i);
		if ((c >= '0') && (c <= '9'))
		{
			return i;
		}
	}
	return ARRAY_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CHARS::GetChar(ASIZE iIndex)
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
void CHARS::Difference(CArrayInt* paiNewToOldIndices, CArrayInt* paiOldToNewIndices, CHARS szOldString)
{
	ASIZE	i;
	ASIZE	j;
	CHARS	cTemp;
	char	cNew;
	char	cOld;
	ASIZE	uiLen;
	ASIZE	iOldLen;

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
				paiOldToNewIndices->SetValue(i, (int)j);
				paiNewToOldIndices->SetValue(j, (int)i);
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
bool CHARS::IsWhitespace(ASIZE iPos, bool bIncludeNewLines)
{
	char	c;

	c = GetChar(iPos);
	return ::IsWhitespace(c, bIncludeNewLines);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHARS::IsWhitespace(void)
{
	ASIZE		i;
	ASIZE		uiLen;

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
void CHARS::SetLength(ASIZE iLength)
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
ASIZE CHARS::FindEndOfLeadingWhitespace(bool bIncludeNewLines)
{
	ASIZE	iLength;
	ASIZE	iStart;

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
ASIZE CHARS::FindStartOfTrailingWhitespace(bool bIncludeNewLines)
{
	ASIZE	iLength;
	ASIZE	iEnd;

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
ASIZE CHARS::StripWhitespace(bool bIncludeNewLines)
{
	ASIZE	iLength;
	ASIZE	iStart;
	ASIZE	iEnd;

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
int CHARS::Compare(CHARS* szOther)
{
	return StringCompare(Text(), szOther->Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CHARS::CompareIgnoreCase(CHARS* szOther)
{
	return StringInsensitiveCompare(Text(), szOther->Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CHARS::Compare(const char* szOther)
{
	return StringCompare(Text(), szOther);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CHARS::CompareIgnoreCase(const char* szOther)
{
	return StringInsensitiveCompare(Text(), szOther);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ASIZE CHARS::Replace(char cFind, char cReplace)
{
	ASIZE	i;
	ASIZE	iLength;
	ASIZE	iCount;

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
ASIZE CHARS::Replace(const char* szFind, const char* szReplace)
{
	ASIZE	iReplaceLen;
	ASIZE	iFindLen;

	if (szFind == NULL)
	{
		return 0;
	}

	iFindLen = CStringLength(szFind);
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
		iReplaceLen = CStringLength(szReplace);
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
ASIZE CHARS::ReplaceWithEqualLength(const char* szFind, const char* szReplace, ASIZE iFindLen)
{
	ASIZE		iCount;
	ASIZE		iIndex;

	iCount = 0;
	iIndex = Find(0, szFind);
	while (iIndex != ARRAY_NOT_FOUND)
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
ASIZE CHARS::ReplaceWithShorter(const char* szFind, const char* szReplace, ASIZE iReplaceLen, ASIZE iFindLen, ASIZE iDifference)
{
	ASIZE		iCount;
	ASIZE		iIndex;
	ASIZE		iTotalDifference;
	ASIZE		iOldIndex;
	ASIZE		iDestPos;
	ASIZE		iSize;
	ASIZE		iEnd;

	//Remember that iDifference is Positive!
	iCount = 0;
	iIndex = Find(0, szFind);
	if (iIndex == ARRAY_NOT_FOUND)
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
				ARRAY_MEMMOVE(mcText.Get(iDestPos), mcText.Get(iOldIndex + iFindLen), iSize - 1);
			}
			Overwrite(iIndex - (iCount * iDifference), szReplace);
		}
		else
		{
			if (iCount != 0)
			{
				ARRAY_MEMMOVE(mcText.Get(iDestPos), mcText.Get(iOldIndex + iFindLen), iSize);
			}
		}
		iCount++;
		iOldIndex = iIndex;
		iDestPos = iOldIndex + iReplaceLen - (iDifference * (iCount - 1));
		iIndex = Find(iIndex + iFindLen, szFind);
		if (iIndex == ARRAY_NOT_FOUND)
		{
			iTotalDifference = iDifference * iCount;
			iEnd = Length() - iTotalDifference;
			iSize = Length() - iOldIndex - iFindLen;
			if (iEnd != iDestPos)
			{
				if (iCount != 0)
				{
					ARRAY_MEMMOVE(mcText.Get(iDestPos), mcText.Get(iOldIndex + iFindLen), iSize);
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
ASIZE CHARS::ReplaceWithLonger(const char* szFind, const char* szReplace, ASIZE iFindLen, ASIZE iDifference)
{
	ASIZE		iCount;
	ASIZE		iIndex;
	ASIZE		i;
	ASIZE		iTotalDifference;
	ASIZE		iOldIndex;

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
		ARRAY_MEMMOVE(mcText.Get(iIndex + iFindLen + (iDifference * (i + 1))), mcText.Get(iIndex + iFindLen), iOldIndex - iIndex);
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
void CHARS::Overwrite(ASIZE iIndex, const char* szReplace)
{
	ASIZE		i;
	ASIZE		j;
	ASIZE		uiLen;

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
void CHARS::Reverse(void)
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
void CHARS::Clear(void)
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
CHARS* CHARS::AppendData(const char* szData, ASIZE iMaxLength)
{
	ASIZE	i;
	ASIZE	iLength;

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
CHARS* CHARS::AppendData(const char* szData, ASIZE iDataLength, ASIZE iMaxLength)
{
	ASIZE	i;
	ASIZE	iLength;
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
bool CHARS::AppendData2(const char* szData, ASIZE iDataLength)
{
	ASIZE	i;
	ASIZE	iPrintable;
	uint8	c;
	float	fPrintable;
	CHARS	szString;

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
CHARS* CHARS::AppendPointer(void* pv)
{
	Append("0x");
	return AppendHexHiLo(&pv, sizeof(void*));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHARS::AppendFlag(uint32 msFlags, uint32 uiFlag, const char* szFlagName, bool bAppendComma)
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
ASIZE CHARS::CountNewLines(void)
{
	return ::CountNewLines(mcText.GetData(), mcText.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ASIZE CHARS::Count(char c)
{
	ASIZE	i;
	char	cAt;
	ASIZE	iCount;
	ASIZE	uiLen;

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
void CHARS::LowerCase(void)
{
	ToLower(mcText.GetData());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHARS::UpperCase(void)
{
	ToUpper(mcText.GetData());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHARS::ReplaceNonTabsWithSpace(void)
{
	ASIZE	i;
	char	c;
	ASIZE	uiLen;

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
void CHARS::PassifyNewlines(void)
{
	lsize	iNewLen;

	//::PassifyNewlines counts with lsize whatever the array's size type.
	iNewLen = ::PassifyNewlines(mcText.GetData());
	if (iNewLen != MAX_LSIZE)
	{
		mcText.Resize((ASIZE)iNewLen + 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CHARS::FindLineContaining(const char* szPosition, ASIZE* piLineNumber)
{
	ASIZE	i;
	ASIZE	iNumLineFeeds;
	ASIZE	uiLen;
	char*	szString;
	char*	szStartOfLine;
	char*	szCurrent;

	uiLen = Length();
	iNumLineFeeds = 0;
	szString = Text();
	szStartOfLine = Text();
	for (i = 0; i < uiLen; i++)
	{
		szCurrent = &szString[i];
		if (*szCurrent == '\n')
		{
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
void CHARS::MakeCPlusPlus(void)
{
	ASIZE	iIndex;
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
bool CHARS::WriteString(CFileWriter* pcWriter)
{
	ARRAY_CHAR c;

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
bool CHARS::ReadString(CFileReader* pcReader)
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
bool CHARS::ReadChars(CFileReader* pcReader)
{
	ASIZE	iLength;

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
void CHARS::CleanIfEmpty(void)
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
void CHARS::InitEmpty(void)
{
	mcText.Fake(gszEmptyString, 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHARS::IsFakeEmpty(void)
{
	return mcText.GetData() == gszEmptyString;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHARS::Unfake(void)
{
	mcText.Init();
	mcText.Add('\0');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHARS::UnfakeIfFakeEmpty(void)
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
char* CHARS::CopyIntoBuffer(char* szDest, ASIZE iDestLength)
{
	ASIZE		iLength;

	if (iDestLength == ARRAY_NOT_FOUND)
	{
		StrCpySafe(szDest, Text(), iDestLength);
		return szDest;
	}
	else if (iDestLength == 0)
	{
		return szDest;
	}
	else
	{
		iLength = Length();
		if (iLength < iDestLength)
		{
			ARRAY_MEMCPY(szDest, Text(), iLength + 1);
		}
		else
		{
			ARRAY_MEMCPY(szDest, Text(), iDestLength - 2);
			szDest[iDestLength - 1] = '\0';
		}
		return szDest;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHARS::Dump(void)
{
	CHARS	szString;
	ASIZE	i;
	ASIZE	uiLen;

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
void CHARS::Dump(const char* szString)
{
	EngineOutput(szString);
}
