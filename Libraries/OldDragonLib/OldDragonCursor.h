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
#ifndef __OLD_DRAGON_CURSOR_H__
#define __OLD_DRAGON_CURSOR_H__
#include "BaseLib/PrimitiveTypes.h"


//The tags in front of values in the 2009 and Max files.  No source for the code that wrote them survives: their meanings
//come from matching the bytes against the order fields are saved in.
#define OLD_DRAGON_TAG_DATA		0x01
#define OLD_DRAGON_TAG_CHARS	0x03
#define OLD_DRAGON_TAG_ARRAY	0x04
#define OLD_DRAGON_TAG_SIMPLE	0x07
#define OLD_DRAGON_TAG_INT		0x0e
#define OLD_DRAGON_TAG_FLOAT	0x0f
#define OLD_DRAGON_TAG_BOOL		0x13


//Reads little endian values from a block of memory.  Once a read runs off the end, or finds the wrong tag, every later
//read returns zero or NULL and Failed returns true, so a reader can check once after a run of reads.
class COldDragonCursor
{
protected:
	const uint8*	mpuiPos;
	const uint8*	mpuiEnd;
	bool			mbFailed;

public:
	void			Init(const uint8* puiData, size uiSize);

	bool			Failed(void);
	size			Remaining(void);
	void			Fail(void);

	uint32			ReadUint32(void);
	int32			ReadInt32(void);
	float32			ReadFloat(void);
	const uint8*	ReadBytes(size uiSize);
	void			ReadName(char* szDest, size uiDestSize, size uiFieldSize);

	uint32			PeekTag(void);
	int32			ReadTaggedInt(void);
	float32			ReadTaggedFloat(void);
	bool			ReadTaggedBool(void);
	const uint8*	ReadTaggedData(size* puiSize);
	void			ReadTaggedName(char* szDest, size uiDestSize);
	void			ReadTaggedChars(char* szDest, size uiDestSize);
	const uint8*	ReadTaggedArray(size* puiElementSize, size* puiNumElements);
	const uint8*	ReadTaggedArrayAugust(size* puiElementSize, size* puiNumElements);
	const uint8*	ReadTaggedSimple(size uiElementSize, size* puiNumElements);
	const uint8*	ReadTaggedSimple2009(size uiElementSize, size* puiNumElements);
};


void	CopyOldDragonName(char* szDest, size uiDestSize, const uint8* puiSource, size uiSourceSize);


#endif // __OLD_DRAGON_CURSOR_H__

