/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela OldDragonLib

Codaphela OldDragonLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela OldDragonLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela OldDragonLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "OldDragonCursor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COldDragonCursor::Init(const uint8* puiData, size uiSize)
{
	mpuiPos = puiData;
	mpuiEnd = puiData + uiSize;
	mbFailed = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool COldDragonCursor::Failed(void)
{
	return mbFailed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size COldDragonCursor::Remaining(void)
{
	return (size)(mpuiEnd - mpuiPos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COldDragonCursor::Fail(void)
{
	mbFailed = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const uint8* COldDragonCursor::ReadBytes(size uiSize)
{
	const uint8*	puiBytes;

	if (mbFailed || (uiSize > Remaining()))
	{
		mbFailed = true;
		return NULL;
	}
	puiBytes = mpuiPos;
	mpuiPos += uiSize;
	return puiBytes;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 COldDragonCursor::ReadUint32(void)
{
	const uint8*	puiBytes;
	uint32			uiValue;

	puiBytes = ReadBytes(sizeof(uint32));
	if (puiBytes == NULL)
	{
		return 0;
	}
	memcpy(&uiValue, puiBytes, sizeof(uint32));
	return uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 COldDragonCursor::ReadInt32(void)
{
	return (int32)ReadUint32();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 COldDragonCursor::ReadFloat(void)
{
	uint32	uiValue;
	float32	fValue;

	uiValue = ReadUint32();
	memcpy(&fValue, &uiValue, sizeof(float32));
	return fValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COldDragonCursor::ReadName(char* szDest, size uiDestSize, size uiFieldSize)
{
	const uint8*	puiBytes;

	puiBytes = ReadBytes(uiFieldSize);
	CopyOldDragonName(szDest, uiDestSize, puiBytes, uiFieldSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 COldDragonCursor::PeekTag(void)
{
	uint32	uiTag;

	if (mbFailed || (Remaining() < sizeof(uint32)))
	{
		return 0;
	}
	memcpy(&uiTag, mpuiPos, sizeof(uint32));
	return uiTag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 COldDragonCursor::ReadTaggedInt(void)
{
	if (ReadUint32() != OLD_DRAGON_TAG_INT)
	{
		mbFailed = true;
		return 0;
	}
	return ReadInt32();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 COldDragonCursor::ReadTaggedFloat(void)
{
	if (ReadUint32() != OLD_DRAGON_TAG_FLOAT)
	{
		mbFailed = true;
		return 0.0f;
	}
	return ReadFloat();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool COldDragonCursor::ReadTaggedBool(void)
{
	if (ReadUint32() != OLD_DRAGON_TAG_BOOL)
	{
		mbFailed = true;
		return false;
	}
	return ReadUint32() != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const uint8* COldDragonCursor::ReadTaggedData(size* puiSize)
{
	uint32	uiSize;

	*puiSize = 0;
	if (ReadUint32() != OLD_DRAGON_TAG_DATA)
	{
		mbFailed = true;
		return NULL;
	}
	uiSize = ReadUint32();
	*puiSize = uiSize;
	return ReadBytes(uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The 2009 files keep an object's name in a fixed block of data.  The
// debug allocator's 0xfd fill follows the terminator.
//
//////////////////////////////////////////////////////////////////////////
void COldDragonCursor::ReadTaggedName(char* szDest, size uiDestSize)
{
	const uint8*	puiName;
	size			uiSize;

	puiName = ReadTaggedData(&uiSize);
	CopyOldDragonName(szDest, uiDestSize, puiName, uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
// A CChars: its tag, then an array of char.
//
//////////////////////////////////////////////////////////////////////////
void COldDragonCursor::ReadTaggedChars(char* szDest, size uiDestSize)
{
	const uint8*	puiChars;
	size			uiElementSize;
	size			uiNumElements;

	if (ReadUint32() != OLD_DRAGON_TAG_CHARS)
	{
		mbFailed = true;
	}
	puiChars = ReadTaggedArray(&uiElementSize, &uiNumElements);
	if (uiElementSize != 1)
	{
		mbFailed = true;
		puiChars = NULL;
	}
	CopyOldDragonName(szDest, uiDestSize, puiChars, uiNumElements);
}


//////////////////////////////////////////////////////////////////////////
//
// Tag, element size, number of elements, chunk size, elements.
//
//////////////////////////////////////////////////////////////////////////
const uint8* COldDragonCursor::ReadTaggedArray(size* puiElementSize, size* puiNumElements)
{
	uint32	uiElementSize;
	uint32	uiNumElements;
	uint64	ulliByteSize;

	*puiElementSize = 0;
	*puiNumElements = 0;
	if (ReadUint32() != OLD_DRAGON_TAG_ARRAY)
	{
		mbFailed = true;
		return NULL;
	}
	uiElementSize = ReadUint32();
	uiNumElements = ReadUint32();
	ReadUint32();

	ulliByteSize = (uint64)uiElementSize * (uint64)uiNumElements;
	if (ulliByteSize > (uint64)Remaining())
	{
		mbFailed = true;
		return NULL;
	}
	*puiElementSize = uiElementSize;
	*puiNumElements = uiNumElements;
	return ReadBytes((size)ulliByteSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The August 2009 files wrote more of the array object: tag, element
// size, element size again, chunk size, number of elements, number
// allocated, a stale pointer, elements.
//
//////////////////////////////////////////////////////////////////////////
const uint8* COldDragonCursor::ReadTaggedArrayAugust(size* puiElementSize, size* puiNumElements)
{
	uint32	uiElementSize;
	uint32	uiNumElements;
	uint64	ulliByteSize;

	*puiElementSize = 0;
	*puiNumElements = 0;
	if (ReadUint32() != OLD_DRAGON_TAG_ARRAY)
	{
		mbFailed = true;
		return NULL;
	}
	uiElementSize = ReadUint32();
	if (ReadUint32() != uiElementSize)
	{
		mbFailed = true;
		return NULL;
	}
	ReadUint32();
	uiNumElements = ReadUint32();
	ReadUint32();
	ReadUint32();

	ulliByteSize = (uint64)uiElementSize * (uint64)uiNumElements;
	if (ulliByteSize > (uint64)Remaining())
	{
		mbFailed = true;
		return NULL;
	}
	*puiElementSize = uiElementSize;
	*puiNumElements = uiNumElements;
	return ReadBytes((size)ulliByteSize);
}


//////////////////////////////////////////////////////////////////////////
//
// Tag, number of elements, a stale pointer, elements.  The element size
// isn't in the file, so the caller has to know it.
//
//////////////////////////////////////////////////////////////////////////
const uint8* COldDragonCursor::ReadTaggedSimple(size uiElementSize, size* puiNumElements)
{
	uint32	uiNumElements;
	uint64	ulliByteSize;

	*puiNumElements = 0;
	if (ReadUint32() != OLD_DRAGON_TAG_SIMPLE)
	{
		mbFailed = true;
		return NULL;
	}
	uiNumElements = ReadUint32();
	ReadUint32();

	ulliByteSize = (uint64)uiElementSize * (uint64)uiNumElements;
	if (ulliByteSize > (uint64)Remaining())
	{
		mbFailed = true;
		return NULL;
	}
	*puiNumElements = uiNumElements;
	return ReadBytes((size)ulliByteSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CopyOldDragonName(char* szDest, size uiDestSize, const uint8* puiSource, size uiSourceSize)
{
	size	i;

	i = 0;
	if (puiSource != NULL)
	{
		while ((i < uiSourceSize) && (i + 1 < uiDestSize) && (puiSource[i] != 0))
		{
			szDest[i] = (char)puiSource[i];
			i++;
		}
	}
	szDest[i] = 0;
}

