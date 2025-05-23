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
#ifndef __CHUNK_INDEX_H__
#define __CHUNK_INDEX_H__
#include "ArrayTemplate.h"
#include "AbstractFile.h"
#include "ChunkFileName.h"


struct SChunkIndex
{
	chunkName	miName;
	filePos		miChunkSize;	//Size of data in this chunk
	filePos		miChunkDataPos;	//Position of chunk data in file

	void Init(filePos iChunkDataPos, chunkName iName, filePos iChunkSize);
};


typedef CArrayTemplate<SChunkIndex>	CChunkIndexArray;


class CChunkIndex
{
protected:
	chunkName	miSearchName;
	size		miCurrChunkNum;

public:
	CChunkIndexArray	mcChunkIndices;

	void	ReadInit(void);
	void	WriteInit(void);
	void	Kill(void);
	uint32	FindFirstChunkWithName(chunkName iName);
	uint32	FindNextChunkWithName(void);
	size	GetNumChunksWithName(chunkName iName);
};


#endif // __CHUNK_INDEX_H__

