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
#ifndef __OLD_DRAGON_FILE_H__
#define __OLD_DRAGON_FILE_H__
#include "BaseLib/ArrayTemplate.h"


//Every layout that used the .DRG extension.  All are little endian with 32 bit sizes and positions.
enum EOldDragonVersion
{
	ODV_Unknown,

	//Today's files, which aren't old: BaseLib's CChunkFile and StandardLib read them.
	ODV_Current,

	//Into Oblivion (about 2003).  A count of chunks, then a linked list of {name, next, size}.  See IOX2021\code.
	ODV_IOX,

	//Viewer V7 beta 2009.8.9.  No file header.  12 byte chunk headers {name, size, index}.  Names are class numbers.
	ODV_August2009,

	//Viewer V7 beta 2009.9.20.  As August, but the headers have grown an MD5 hash and a magic number.
	ODV_September2009,

	//The 3ds Max 2009 exporter's files.  A file header, 32 byte chunk headers and a table of class names at the end.
	ODV_Max,
};


struct SOldDragonChunk
{
	uint32	uiName;
	int32	iParent;		//Index of the parent in the file's chunks, -1 for the root.
	size	uiHeaderPos;
	size	uiDataPos;
	size	uiDataSize;
	bool	bUnclosed;		//Never finished by the writer: uiDataSize is only how far it is to the next chunk.
};


struct SOldDragonChunkName
{
	uint32	uiName;
	char	szName[64];
};


//A .DRG file in memory: which layout it is and where its chunks are.  COldDragonScene is what's in the chunks.
class COldDragonFile
{
protected:
	uint8*									mpuiData;
	size									muiSize;
	EOldDragonVersion						meVersion;
	bool									mbDamaged;
	CArrayTemplate<SOldDragonChunk>			macChunks;
	CArrayTemplate<SOldDragonChunkName>		macNames;

public:
	void				_Init(void);
	bool				Init(const char* szFilename);
	void				Kill(void);

	EOldDragonVersion	GetVersion(void);
	bool				IsDamaged(void);

	size				NumChunks(void);
	SOldDragonChunk*	GetChunk(size uiIndex);
	const char*			GetChunkName(SOldDragonChunk* psChunk);
	const uint8*		GetChunkData(SOldDragonChunk* psChunk);
	size				FindChild(size uiParent, uint32 uiName);

protected:
	bool				ReadWholeFile(const char* szFilename);
	EOldDragonVersion	Identify(void);
	bool				WalkIOX(void);
	bool				WalkIndexed(size uiHeaderPos, size uiHeaderSize, int32 iParent, size uiDepth);
	bool				ReadNames(size uiPos);
	bool				SalvageMax(size uiEnd);
	bool				Contains(size uiPos, size uiLength);
	uint32				Uint32At(size uiPos);
	const char*			FindName(uint32 uiName);
	bool				AddChunk(uint32 uiName, int32 iParent, size uiHeaderPos, size uiHeaderSize, size uiDataSize, bool bUnclosed);
};


const char*	OldDragonVersionName(EOldDragonVersion eVersion);


#endif // __OLD_DRAGON_FILE_H__

