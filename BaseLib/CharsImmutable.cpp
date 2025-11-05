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
#include "FileWriter.h"
#include "FileReader.h"
#include "DebugOutput.h"
#include "CharsImmutable.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCharsImmutable::Init(char* sz)
{
	Init(sz, &gcSystemAllocator);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCharsImmutable::Init(char* sz, CMallocator* pcMalloc)
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
void CCharsImmutable::Init(char* sz, size uiLength)
{
	Init(sz, uiLength, &gcSystemAllocator);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCharsImmutable::Init(char* sz, size uiLength, CMallocator* pcMalloc)
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
void CCharsImmutable::Init(void)
{
	mbMallocated = false;
	memset(muString.sz, 0, MAX_EMBEDDED_CHARS);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCharsImmutable::Init(size uiLength, CMallocator* pcMalloc)
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
void CCharsImmutable::Dump(void)
{
	EngineOutput(Text());
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
		Init(uiLength - 1, pcMalloc);
	}
	else
	{
		Init();
	}

	bResult = pcReader->ReadStringChars(Text(), uiLength);
	return bResult;
}

