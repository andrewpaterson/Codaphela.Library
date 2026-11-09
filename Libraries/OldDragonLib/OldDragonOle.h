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
#ifndef __OLD_DRAGON_OLE_H__
#define __OLD_DRAGON_OLE_H__
#include "BaseLib/PrimitiveTypes.h"


//An OLE compound file: a little file system of named streams inside a file, which is what a 3ds Max .max file is.
class COldDragonOle
{
protected:
	uint8*	mpuiData;
	size	muiSize;
	size	muiSectorSize;
	size	muiMiniSectorSize;
	size	muiMiniCutoff;
	uint32*	mpuiFat;
	size	muiFatEntries;
	uint32*	mpuiMiniFat;
	size	muiMiniFatEntries;
	uint8*	mpuiDirectory;
	size	muiDirectorySize;
	uint8*	mpuiMiniStream;
	size	muiMiniStreamSize;

public:
	void	_Init(void);
	bool	Init(const char* szFilename);
	void	Kill(void);

	//Finds the first stream whose name starts with szPrefix.  *ppuiStream is the caller's to free.
	bool	ReadStream(const char* szPrefix, uint8** ppuiStream, size* puiSize);

protected:
	bool	ReadFile(const char* szFilename);
	bool	ReadFat(void);
	bool	ReadChain(uint32 uiStart, uint8** ppuiDest, size* puiSize);
	bool	ReadMiniChain(uint32 uiStart, uint8* puiDest, size uiSize);
	uint32	Uint32At(size uiPos);
	bool	DirectoryNameStartsWith(const uint8* puiEntry, const char* szPrefix);
};


#endif // __OLD_DRAGON_OLE_H__

