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
	BOOL				mbLastHashCheck;
	CMapStringInt		mmsziNames;
	SChunkFileHeader	msHeader;
	int					miLastName;
	
	BOOL	__PrivateReadChunkBegin(void);
	BOOL	__PrivateReadChunkIndex(filePos iIndexPos, CChunkIndex* pcIndex);

public:
	void	_Init(void);
	void	Init(CAbstractFile*	pcFile);
	void	Kill(void);

	BOOL	ReadOpen(void);
	BOOL	ReadClose(void);

	BOOL	WriteOpen(void);
	BOOL	WriteOpen(int iUserID);
	BOOL	WriteClose(void);

	int		FindFirstChunkWithName(char* szName);
	int		FindNextChunkWithName(void);
	int		GetNumChunks(void);
	void*	GetMD5Hash(void);
	BOOL	ContainsChunks(void);

	BOOL	ReadChunkBegin(int iChunkIndex);
	BOOL	ReadChunkBegin(char* szName);
	BOOL	ReadChunkEnd(void);
	BOOL	ReadChunkEnd(int iNumChunksToEnd);
	BOOL	ReadChunkEndAll(void);

	BOOL	WriteChunkBegin(void);
	BOOL	WriteChunkEnd(char* szChunkName);

	int		StackDepth(void);
	filePos	ChunkSize(void);
	filePos	ChunkStart(void);
	
	char*	GetTailChunkNameForIndex(int iIndex);
	BOOL	SeekStart(void);

protected:
	int		FindFirstChunkWithID(int iName);
	int		GetNumChunksWithID(int iName);
	BOOL	ReadChunkNames(void);
	BOOL	WriteChunkNames(void);
	BOOL	WriteChunkEnd(int iChunkName);
	BOOL	WriteBasic(const void* pvSource, filePos iSize);
	int		GetTailChunkNameIndex(int iChunkNum);
};


#endif // !__CHUNK_FILE_H__

