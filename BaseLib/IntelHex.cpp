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
#include "TextFile.h"
#include "IntelHex.h"


#define INTEL_HEX_DATA				0x00
#define INTEL_HEX_END				0x01
#define INTEL_HEX_SEGMENT_ADDRESS	0x02
#define INTEL_HEX_SEGMENT_START		0x03
#define INTEL_HEX_LINEAR_ADDRESS	0x04
#define INTEL_HEX_LINEAR_START		0x05


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIntelHex::Init(void)
{
	macSegments.Init();
	mszError.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIntelHex::Kill(void)
{
	Clear();
	macSegments.Kill();
	mszError.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIntelHex::Clear(void)
{
	size	i;

	for (i = 0; i < macSegments.NumElements(); i++)
	{
		macSegments.Get(i)->cBytes.Kill();
	}
	macSegments.Kill();
	macSegments.Init();
	mszError.Kill();
	mszError.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIntelHex::Read(const char* szFilename)
{
	CTextFile	cFile;
	bool		bResult;

	cFile.Init();
	if (!cFile.Read(szFilename))
	{
		cFile.Kill();
		Clear();
		mszError.Append("Could not read ")->Append(szFilename);
		return false;
	}
	bResult = Parse(cFile.Text());
	cFile.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces the bytes with those of szText, one record a line.  Blank lines
// and the line ends of any platform are allowed.
//
//////////////////////////////////////////////////////////////////////////
bool CIntelHex::Parse(const char* szText)
{
	const char*	szLine;
	size		uiLength;
	uint32		uiLine;
	uint32		uiBase;
	bool		bEnd;

	Clear();
	uiBase = 0;
	bEnd = false;
	uiLine = 1;
	szLine = szText;
	while (*szLine)
	{
		uiLength = 0;
		while ((szLine[uiLength] != '\0') && (szLine[uiLength] != '\n') && (szLine[uiLength] != '\r'))
		{
			uiLength++;
		}
		while ((uiLength > 0) && ((szLine[uiLength - 1] == ' ') || (szLine[uiLength - 1] == '\t')))
		{
			uiLength--;
		}
		if (uiLength != 0)
		{
			if (bEnd)
			{
				return Error(uiLine, "Record after the end of file record");
			}
			if (!ParseRecord(szLine, uiLength, uiLine, &uiBase, &bEnd))
			{
				return false;
			}
		}

		while ((*szLine != '\0') && (*szLine != '\n') && (*szLine != '\r'))
		{
			szLine++;
		}
		if ((szLine[0] == '\r') && (szLine[1] == '\n'))
		{
			szLine++;
		}
		if (*szLine)
		{
			szLine++;
		}
		uiLine++;
	}
	if (!bEnd)
	{
		return Error(uiLine, "No end of file record");
	}
	return Join();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int IntelHexDigit(char c)
{
	if ((c >= '0') && (c <= '9'))
	{
		return c - '0';
	}
	if ((c >= 'A') && (c <= 'F'))
	{
		return c - 'A' + 10;
	}
	if ((c >= 'a') && (c <= 'f'))
	{
		return c - 'a' + 10;
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// :LLAAAATTDD...CC
//
//////////////////////////////////////////////////////////////////////////
bool CIntelHex::ParseRecord(const char* szLine, size uiLength, uint32 uiLine, uint32* puiBase, bool* pbEnd)
{
	uint8		auiBytes[260];
	size		uiBytes;
	size		i;
	int			iHigh;
	int			iLow;
	uint8		uiSum;
	uint8		uiCount;
	uint32		uiOffset;
	uint8		uiType;
	uint64		ulliAddress;

	if (szLine[0] != ':')
	{
		return Error(uiLine, "A record must start with ':'");
	}
	if (((uiLength - 1) % 2 != 0) || (uiLength < 11) || (uiLength > 1 + 2 * 260))
	{
		return Error(uiLine, "Bad record length");
	}
	uiBytes = (uiLength - 1) / 2;
	uiSum = 0;
	for (i = 0; i < uiBytes; i++)
	{
		iHigh = IntelHexDigit(szLine[1 + 2 * i]);
		iLow = IntelHexDigit(szLine[2 + 2 * i]);
		if ((iHigh < 0) || (iLow < 0))
		{
			return Error(uiLine, "Bad hex digit");
		}
		auiBytes[i] = (uint8)((iHigh << 4) | iLow);
		uiSum += auiBytes[i];
	}
	uiCount = auiBytes[0];
	if (uiBytes != (size)uiCount + 5)
	{
		return Error(uiLine, "Byte count does not match the record");
	}
	if (uiSum != 0)
	{
		return Error(uiLine, "Bad checksum");
	}

	uiOffset = ((uint32)auiBytes[1] << 8) | auiBytes[2];
	uiType = auiBytes[3];
	switch (uiType)
	{
	case INTEL_HEX_DATA:
		ulliAddress = (uint64)*puiBase + uiOffset;
		if (ulliAddress + uiCount > 0x100000000ULL)
		{
			return Error(uiLine, "Data past the end of a 32 bit address");
		}
		Write((uint32)ulliAddress, &auiBytes[4], uiCount);
		return true;

	case INTEL_HEX_END:
		*pbEnd = true;
		return true;

	case INTEL_HEX_SEGMENT_ADDRESS:
	case INTEL_HEX_LINEAR_ADDRESS:
		if (uiCount != 2)
		{
			return Error(uiLine, "An address record must have 2 bytes");
		}
		*puiBase = (((uint32)auiBytes[4] << 8) | auiBytes[5]) << ((uiType == INTEL_HEX_SEGMENT_ADDRESS) ? 4 : 16);
		return true;

	case INTEL_HEX_SEGMENT_START:
	case INTEL_HEX_LINEAR_START:
		return true;

	default:
		return Error(uiLine, "Unknown record type");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Records usually follow each other, so a record that starts where the last
// segment ends is added to it.
//
//////////////////////////////////////////////////////////////////////////
void CIntelHex::Write(uint32 uiAddress, uint8* puiBytes, size uiLength)
{
	SIntelHexSegment*	psSegment;
	size				uiOld;

	if (uiLength == 0)
	{
		return;
	}
	psSegment = macSegments.Tail();
	if ((psSegment == NULL) || ((uint64)psSegment->uiAddress + psSegment->cBytes.NumElements() != uiAddress))
	{
		psSegment = macSegments.Add();
		psSegment->uiAddress = uiAddress;
		psSegment->cBytes.Init();
	}
	uiOld = psSegment->cBytes.NumElements();
	psSegment->cBytes.Resize(uiOld + uiLength);
	memcpy(psSegment->cBytes.Get(uiOld), puiBytes, uiLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int CompareIntelHexSegment(const void* pvLeft, const void* pvRight)
{
	SIntelHexSegment*	psLeft;
	SIntelHexSegment*	psRight;

	psLeft = (SIntelHexSegment*)pvLeft;
	psRight = (SIntelHexSegment*)pvRight;
	if (psLeft->uiAddress < psRight->uiAddress)
	{
		return -1;
	}
	if (psLeft->uiAddress > psRight->uiAddress)
	{
		return 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Sorts the segments by address and joins those that touch.
//
//////////////////////////////////////////////////////////////////////////
bool CIntelHex::Join(void)
{
	size				i;
	SIntelHexSegment*	psLast;
	SIntelHexSegment*	psSegment;
	uint64				ulliLastEnd;
	uint32				uiAddress;

	macSegments.QuickSort(CompareIntelHexSegment);
	i = 1;
	while (i < macSegments.NumElements())
	{
		psLast = macSegments.Get(i - 1);
		psSegment = macSegments.Get(i);
		ulliLastEnd = (uint64)psLast->uiAddress + psLast->cBytes.NumElements();
		if (ulliLastEnd > psSegment->uiAddress)
		{
			uiAddress = psSegment->uiAddress;
			Clear();
			mszError.Append("Address ")->AppendHexHiLo(&uiAddress, 4)->Append(" is given more than once");
			return false;
		}
		if (ulliLastEnd == psSegment->uiAddress)
		{
			psLast->cBytes.InsertArrayAfterEnd(&psSegment->cBytes);
			psSegment->cBytes.Kill();
			macSegments.RemoveAt(i, true);
		}
		else
		{
			i++;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIntelHex::Error(uint32 uiLine, const char* szMessage)
{
	Clear();
	mszError.Append("Line ")->Append(uiLine)->Append(": ")->Append(szMessage);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CIntelHex::GetError(void)
{
	return mszError.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIntelHex::NumSegments(void)
{
	return macSegments.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIntelHexSegment* CIntelHex::GetSegment(size uiIndex)
{
	return macSegments.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIntelHex::IsEmpty(void)
{
	return macSegments.NumElements() == 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The lowest address with a byte (0 when there are none).
//
//////////////////////////////////////////////////////////////////////////
uint32 CIntelHex::GetLowAddress(void)
{
	if (macSegments.NumElements() == 0)
	{
		return 0;
	}
	return macSegments.Get(0)->uiAddress;
}


//////////////////////////////////////////////////////////////////////////
//
// The address after the highest byte (0 when there are none).
//
//////////////////////////////////////////////////////////////////////////
uint64 CIntelHex::GetEndAddress(void)
{
	SIntelHexSegment*	psSegment;

	psSegment = macSegments.Tail();
	if (psSegment == NULL)
	{
		return 0;
	}
	return (uint64)psSegment->uiAddress + psSegment->cBytes.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIntelHex::Get(uint32 uiAddress, uint8* puiByte)
{
	size				i;
	SIntelHexSegment*	psSegment;

	for (i = 0; i < macSegments.NumElements(); i++)
	{
		psSegment = macSegments.Get(i);
		if ((uiAddress >= psSegment->uiAddress) && ((uint64)uiAddress < (uint64)psSegment->uiAddress + psSegment->cBytes.NumElements()))
		{
			*puiByte = psSegment->cBytes.GetValue(uiAddress - psSegment->uiAddress);
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The uiLength bytes from uiAddress, with uiFill where the file has none.
//
//////////////////////////////////////////////////////////////////////////
void CIntelHex::Fill(uint32 uiAddress, uint32 uiLength, uint8 uiFill, uint8* puiDest)
{
	size				i;
	SIntelHexSegment*	psSegment;
	uint64				ulliStart;
	uint64				ulliEnd;
	uint64				ulliSegmentEnd;

	memset(puiDest, uiFill, uiLength);
	ulliEnd = (uint64)uiAddress + uiLength;
	for (i = 0; i < macSegments.NumElements(); i++)
	{
		psSegment = macSegments.Get(i);
		ulliSegmentEnd = (uint64)psSegment->uiAddress + psSegment->cBytes.NumElements();
		ulliStart = (psSegment->uiAddress > uiAddress) ? psSegment->uiAddress : uiAddress;
		if (ulliSegmentEnd > ulliEnd)
		{
			ulliSegmentEnd = ulliEnd;
		}
		if (ulliStart < ulliSegmentEnd)
		{
			memcpy(&puiDest[ulliStart - uiAddress], psSegment->cBytes.Get((size)(ulliStart - psSegment->uiAddress)), (size)(ulliSegmentEnd - ulliStart));
		}
	}
}
