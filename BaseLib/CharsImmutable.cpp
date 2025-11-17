/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include <string.h>
#include "SystemAllocator.h"
#include "GlobalMemory.h"
#include "StringHelper.h"
#include "FileWriter.h"
#include "FileReader.h"
#include "DebugOutput.h"
#include "CharsImmutable.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCharsImmutable::Init(const char* sz)
{
	Init(sz, &gcSystemAllocator);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCharsImmutable::Init(const char* sz, CMallocator* pcMalloc)
{
	size	uiLength;

	uiLength = strlen(sz);

	if (uiLength >= MAX_EMBEDDED_CHARS)
	{
		mbMallocated = true;
		muString.sStringAlloc.pcMalloc = pcMalloc;
		muString.sStringAlloc.szString = (char*)pcMalloc->Malloc(uiLength + 1);
		muString.sStringAlloc.uiLength = uiLength;
		memcpy(muString.sStringAlloc.szString, sz, uiLength + 1);
	}
	else
	{
		mbMallocated = false;
		strcpy(muString.sz, sz);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCharsImmutable::Init(const char* sz, size uiLength)
{
	Init(sz, uiLength, &gcSystemAllocator);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCharsImmutable::Init(const char* sz, size uiLength, CMallocator* pcMalloc)
{
	if (uiLength >= MAX_EMBEDDED_CHARS)
	{
		mbMallocated = true;
		muString.sStringAlloc.pcMalloc = pcMalloc;
		muString.sStringAlloc.szString = (char*)pcMalloc->Malloc(uiLength + 1);
		muString.sStringAlloc.uiLength = uiLength;
		memcpy(muString.sStringAlloc.szString, sz, uiLength);
		muString.sStringAlloc.szString[uiLength] = '\0';
	}
	else
	{
		mbMallocated = false;
		memcpy(muString.sz, sz, uiLength);
		muString.sz[uiLength] = '\0';
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCharsImmutable::_Init(void)
{
	//_Init() just exists to make converting from CChars easier.
	Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCharsImmutable::Init(void)
{
	mbMallocated = false;
	memset(muString.sz, 0, MAX_EMBEDDED_CHARS);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCharsImmutable::InitLength(size uiLength, CMallocator* pcMalloc)
{
	mbMallocated = true;

	muString.sStringAlloc.pcMalloc = pcMalloc;
	muString.sStringAlloc.szString = (char*)pcMalloc->Malloc(uiLength + 1);
	muString.sStringAlloc.uiLength = uiLength;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCharsImmutable::InitLength(size uiLength)
{
	InitLength(uiLength, &gcSystemAllocator);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCharsImmutable::Kill(void)
{
	if (mbMallocated)
	{
		muString.sStringAlloc.pcMalloc->Free(muString.sStringAlloc.szString);
		mbMallocated = false;
	}
	muString.sz[0] = '\0';
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CCharsImmutable::Text(void)
{
	if (mbMallocated)
	{
		return muString.sStringAlloc.szString;
	}
	else
	{
		return muString.sz;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CCharsImmutable::Length(void)
{
	if (mbMallocated)
	{
		return muString.sStringAlloc.uiLength;
	}
	else
	{
		return strlen(muString.sz);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCharsImmutable::Set(const char* sz)
{
	CMallocator*	pcMalloc;

	pcMalloc = NULL;
	if (mbMallocated)
	{
		pcMalloc = muString.sStringAlloc.pcMalloc;
		pcMalloc->Free(muString.sStringAlloc.szString);
		mbMallocated = false;
	}

	Init(sz, pcMalloc);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCharsImmutable::Empty(void)
{
	if (Length() == 0)
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCharsImmutable::Dump(void)
{
	EngineOutput(Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCharsImmutable::Equals(const char* szString)
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
bool CCharsImmutable::Equals(const char* szString, size uiLen)
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
bool CCharsImmutable::Equals(CChars* pszString)
{
	return Equals(pszString->Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCharsImmutable::EqualsIgnoreCase(const char* szString)
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
bool CCharsImmutable::EqualsIgnoreCase(CChars* pszString)
{
	return EqualsIgnoreCase(pszString->Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCharsImmutable::Contains(const char* szString)
{
	char* pc;

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
bool CCharsImmutable::ContainsIgnoreCase(const char* szString)
{
	char* pc;

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
bool CCharsImmutable::Contains(char c)
{
	return Find(0, c) != ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCharsImmutable::EndsWith(const char* szString)
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
bool CCharsImmutable::EndsWith(char c)
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
bool CCharsImmutable::EndsWithIgnoreCase(const char* szString)
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
bool CCharsImmutable::StartsWith(const char* szString)
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
bool CCharsImmutable::StartsWithIgnoreCase(const char* szString)
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
bool CCharsImmutable::StartsWith(char c)
{
	size		iLength;

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
size CCharsImmutable::FindFromEnd(const char* szString)
{
	size iOtherLen;

	iOtherLen = strlen(szString);
	return FindFromEnd(Length() - iOtherLen, szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCharsImmutable::FindFromEnd(size iIndex, const char* szString)
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
	} while (i != 0);
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCharsImmutable::FindFromEnd(char c)
{
	if (Length() == 0)
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}

	return FindFromEnd(Length() - 1, c);

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCharsImmutable::FindFromEnd(size iIndex, char c)
{
	size	i;
	char*	szText;

	szText = Text();
	i = iIndex + 1;
	do
	{
		i--;
		if (szText[i] == c)
		{
			return i;
		}
	} while (i != 0);
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCharsImmutable::Find(const char* szString)
{
	return Find(0, szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCharsImmutable::Find(size iIndex, const char* szString)
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
size CCharsImmutable::Find(size iIndex, char c)
{
	size	i;
	size	uiLen;
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
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCharsImmutable::Find(char c)
{
	return Find(0, c);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCharsImmutable::WriteString(CFileWriter* pcWriter)
{
	bool	bResult;
	size	iStrLen;

	iStrLen = Length() + 1;
	bResult = pcWriter->WriteSize(iStrLen);
	ReturnOnFalse(bResult);

	if (mbMallocated)
	{
		bResult = gcMallocators.Write(pcWriter, muString.sStringAlloc.pcMalloc);
		ReturnOnFalse(bResult);
		bResult = pcWriter->WriteData(muString.sStringAlloc.szString, iStrLen);
		return bResult;
	}
	else
	{
		bResult = pcWriter->WriteData(muString.sz, iStrLen);
		return bResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCharsImmutable::ReadString(CFileReader* pcReader)
{
	size			uiLength;
	bool			bResult;
	CMallocator*	pcMalloc;

	bResult = pcReader->ReadStringLength(&uiLength);
	ReturnOnFalse(bResult);

	if (uiLength > MAX_EMBEDDED_CHARS)
	{
		pcMalloc = gcMallocators.Read(pcReader);
		if (pcMalloc == NULL)
		{
			return false;
		}
		InitLength(uiLength - 1, pcMalloc);
	}
	else
	{
		Init();
	}

	bResult = pcReader->ReadStringChars(Text(), uiLength);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCharsImmutable::SubStringEquals(size iStart, const char* szString)
{
	size	i;
	size	j;
	size	uiLen;
	size	iSubLen;
	char*	szText;

	szText = Text();
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
bool CCharsImmutable::SubStringEqualsIgnoreCase(size iStart, const char* szString)
{
	size	i;
	size	j;
	size	uiLen;
	size    c1;
	size    c2;
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

