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
#ifndef __CHUNK_FILE_H__
#define __CHUNK_FILE_H__
#include "Define.h"
#include "ChunkFileHeader.h"
#include "ChunkFileGeneral.h"
#include "FileIO.h"
#include "FileBasic.h"
#include "MapStringInt.h"
#include "ChunkStack.h"


class CChunkFile : public CFileBasic
{
protected:
	CChunkStack			mcChunkStack;
	bool				mbLastHashCheck;
	CMapStringInt		mmsziNames;
	SChunkFileHeader	msHeader;
	chunkName			miLastName;
	
	bool	__PrivateReadChunkBegin(void);
	bool	__PrivateReadChunkIndex(filePos iIndexPos, CChunkIndex* pcIndex);

public:
	void		_Init(void);
	void		Init(CAbstractFile*	pcFile);
	void		Kill(void);

	bool		ReadOpen(void);
	bool		ReadClose(void);

	bool		WriteOpen(void);
	bool		WriteOpen(chunkName iUserID);
	bool		WriteClose(void);

	size		FindFirstChunkWithName(char* szName);
	size		FindNextChunkWithName(void);
	size		GetNumChunks(void);
	void*		GetMD5Hash(void);
	bool		ContainsChunks(void);

	bool		ReadChunkBegin(size iChunkIndex);
	bool		ReadChunkBegin(char* szName);
	bool		ReadChunkEnd(void);
	bool		ReadChunkEnd(size iNumChunksToEnd);
	bool		ReadChunkEndAll(void);

	bool		WriteChunkBegin(void);
	bool		WriteChunkEnd(char* szChunkName);

	size		StackDepth(void);
	filePos		ChunkSize(void);
	filePos		ChunkStart(void);
	
	char*		GetTailChunkNameForIndex(size iIndex);
	bool		SeekStart(void);

protected:
	size		FindFirstChunkWithID(chunkName iName);
	size		GetNumChunksWithID(chunkName iName);
	bool		ReadChunkNames(void);
	bool		WriteChunkNames(void);
	bool		WriteChunkEnd(chunkName iChunkName);
	bool		WriteBasic(const void* pvSource, size uiSize);
	chunkName	GetTailChunkNameIndex(size iChunkNum);
};


#endif // __CHUNK_FILE_H__

