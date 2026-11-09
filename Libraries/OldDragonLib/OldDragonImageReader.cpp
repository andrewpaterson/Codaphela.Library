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
#include "OldDragonImageReader.h"


//////////////////////////////////////////////////////////////////////////
//
// Tries each channel count, looking for a simple array's tag and size
// exactly that far from the end of the chunk.  August 2009 wrote the
// stale pointer before the count and Max wrote it after, so the size can
// be in either place.  It's the size matching that says these are pixels.
//
//////////////////////////////////////////////////////////////////////////
bool ReadOldDragonImagePixels(COldDragonCursor* pcCursor, COldDragonImage* pcImage)
{
	static const int32	aiChannels[] = { 4, 3, 1 };
	size				i;
	uint64				ulliByteSize;
	size				uiRemaining;
	size				uiSize;
	const uint8*		puiBlock;
	uint32				auiTag[3];

	uiRemaining = pcCursor->Remaining();
	puiBlock = pcCursor->ReadBytes(uiRemaining);
	if (puiBlock == NULL)
	{
		return false;
	}

	for (i = 0; i < sizeof(aiChannels) / sizeof(aiChannels[0]); i++)
	{
		ulliByteSize = (uint64)(uint32)pcImage->miWidth * (uint64)(uint32)pcImage->miHeight * (uint64)aiChannels[i];
		if (ulliByteSize + sizeof(auiTag) > (uint64)uiRemaining)
		{
			continue;
		}

		uiSize = (size)ulliByteSize;
		memcpy(auiTag, &puiBlock[uiRemaining - uiSize - sizeof(auiTag)], sizeof(auiTag));
		if ((auiTag[0] == OLD_DRAGON_TAG_SIMPLE) && (((uint64)auiTag[1] == ulliByteSize) || ((uint64)auiTag[2] == ulliByteSize)))
		{
			pcImage->miChannels = aiChannels[i];
			return pcImage->SetPixels(&puiBlock[uiRemaining - uiSize], uiSize);
		}
	}
	return true;
}

