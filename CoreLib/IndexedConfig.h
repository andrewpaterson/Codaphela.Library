/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __INDEX_CONFIG_H__
#define __INDEX_CONFIG_H__
#include "BaseLib/Chars.h"


class CIndexedConfig
{
public:
	//Where the database files are saved.
	char*	mszWorkingDirectory;

	//Several file operations together are durable if they either all succeed or all fail.
	//Setting the database to be durable doubles it's size.
	BOOL	mbDurable;

	//Only update the disk if the new object data is different (using memcmp).
	//This will cause a disk read if the object is not cached.
	BOOL	mbDirtyTesting;

	//Immediately write the object data to disk, even if the object is cached.
	BOOL	mbWriteThrough;

	//For the indices caching settings, See the IndexedHuge.h file
	int		miIndicesSecondLevelWidth;
	int		miIndicesThirdLevelWidth;
	int		miIndicesNumSecondLevelChunks; 
	int		miIndicesNumThirdLevelChunks;

	//If the smart index access has not switched to huge access yet, then the memory
	//access allocates chunks in the size below.
	int		miIndicesMemoryChunkSize;

	//Size of the object cache in bytes.
	int		miObjectsCacheSize;

	//If transient is set then don't write out cached objects to disk on close and
	//remove the database files on close.
	BOOL	mbTransient;

	void	Manual(char* szWorkingDirectory, BOOL bDurable, BOOL bDirtyTesting, BOOL mbWriteThrough, int iIndicesSecondLevelWidth, int iIndicesThirdLevelWidth, int iIndicesNumSecondLevelChunks, int iIndicesNumThirdLevelChunks, int iIndicesMemoryChunkSize, int iObjectsCacheSize, BOOL bTransient);
	void	OptimiseForStreaming(char* szWorkingDirectory);
	void	OptimiseForGameGraph(char* szWorkingDirectory);
	void	OptimiseForTransactions(char* szWorkingDirectory);

	void	SetDurable(BOOL bDurable);
	void	SetDirtyTesting(BOOL bDirtyTesting);
	void	SetWriteThrough(BOOL bWriteThrough);
	void	DisableObjectCaching(void);
	void	SetObjectCacheSize(int iObjectsCacheSize);
	void	SetTransient(BOOL bTransient);
};


#endif // __INDEX_CONFIG_H__
