/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "ChunkFileNames.h"
#include "ChunkFileIndex.h"
#include "FileIO.h"
#include "FileBasic.h"
#include "ArrayTemplate.h"
#include "MapStringInt.h"
#include "MD5.h"
#include "ASCIITree.h"


#define CHUNK_HEADER_MAGIC 0xb4d85f9a


struct SChunkFileHeader
{
	filePos		iChunkNamesPos;
	char		acMD5Hash[16];  //0's if not hashed.
	int			iMagic;

	void		WriteInit(void);
};


class CChunkHeader
{
public:
	int		iName;
	filePos	iChunkSize;
	filePos	iChunkIndexPos;  // -1 if no index.
	char	acMD5Hash[16];  //0's if not hashed.
	int		iMagic;

	void WriteInit(void);
};


class CChunkStackElement
{
public:
	CChunkHeader			sHeader;
	filePos					iChunkHeaderPos;
	BOOL					bContainsChunks;
	CChunkIndex				cChunkIndex;
	SMD5Context				sMD5Context;
};


typedef CArrayTemplate<CChunkStackElement> CChunkStack;


class CChunkFile : public CFileBasic
{
private:
	CChunkStack			mcChunkStack;
	BOOL				mbLastHashCheck;
	CMapStringInt		mmsziNames;
	SChunkFileHeader	msHeader;
	int					miLastName;
	
	BOOL	__PrivateReadChunkBegin(void);
	BOOL	__PrivateReadChunkIndex(filePos iIndexPos, CChunkIndex* pcIndex);

public:
	void	Init(CAbstractFile*	pcFile);
	void	Kill(void);

	BOOL	ReadOpen(void);
	BOOL	ReadClose(void);

	BOOL	WriteOpen(void);
	BOOL	WriteClose(void);

	int		FindFirstChunkWithName(char* szName);
	int		FindFirstChunkWithName(int iName);
	int		FindNextChunkWithName(void);
	int		GetNumChunksWithName(int iName);
	int		GetNumChunks(void);
	int		GetChunkName(int iChunkNum);
	void*	GetMD5Hash(void);
	BOOL	ContainsChunks(void);

	BOOL	ReadChunkBegin(int iChunkNum);  //ChunkIndex not ChunkName
	BOOL	ReadChunkEnd(void);
	BOOL	ReadChunkEnd(int iNumChunksToEnd);

	BOOL	WriteChunkBegin(void);
	BOOL	WriteChunkEnd(char* szChunkName);
	BOOL	WriteChunkEnd(int iChunkName);

protected:
	BOOL	ReadChunkNames(void);
	BOOL	WriteChunkNames(void);
};


#endif // __CHUNK_FILE_H__

