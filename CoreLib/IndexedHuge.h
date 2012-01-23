/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __INDEXED_HUGE_H__
#define __INDEXED_HUGE_H__
#include "IndexDescriptor.h"
#include "BaseLib/ArrayBlock.h"
#include "DurableFile.h"
#include "BaseLib/Numbers.h"


#define HUGE_DESRIPTOR_READ_SIZE	(4 MB)

//  First Level Grouping  ----+-------------------------------------+-----------------------------------+--- ...
//		Groups of: 12         |                                     |                                   |
//						      v                                     v                                   v
//	Second Level Chunks	     0-11                                 12-23                                 .
//		Num Chunks: 2	     /+---------+-----------+-\           /-+---------+---------+-\             
//		Chunk Width: 12	      |         |           |               |         |         |
//						      v         v           v               v         v         v
//						     0-3       4-7         8-11           12-15     16-19     20-23
//	Third Level Chunks	     /-----\    .          /-------\      /-------\   .         .
//		Num Chunks: 3	     . 1 2 .               8 . 10 11      12 . 14 .   
//		Chunk Width: 4
//
//
//	The important numbers are: Second Level Chunk width and Third Level Chunk width (2nd exactly divisible by 3rd)
//	Number of Second Level Chunks and Number of Third Level Chunks (unrelated).
//
//	Technically you don't need the array.  You can just search the 2nd level chunks.
//  


struct SIndexedSecondLevelSearch
{
	int		iArrayIndex;  //Lookup back into the array;
	OIndex	iFirst;
	OIndex	iLast;
	//See __DEBUG_SIndexedSecondLevelSearch for what follows after.
};

typedef CArrayTemplate<SIndexedSecondLevelSearch*> CArrayIndexedSearchPtr;


struct SIndexedThirdLevelSearch
{
	BOOL						bValid;	  //Cached or whatever...
	SIndexedThirdLevelSearch*	psNext;
	SIndexedSecondLevelSearch*	psParent;
	int							iSecondLevelOffset;  //Add the second level 'iFirst' to the offset to get the 'oi'.
	//See __DEBUG_SIndexedThirdLevelSearch for what follows after.
};


struct __DEBUG_SIndexedThirdLevelSearch : SIndexedThirdLevelSearch
{
	CIndexDescriptor	acIndexedDataDescriptor[2];
};
struct __DEBUG_SIndexedSecondLevelSearch : SIndexedSecondLevelSearch
{
	__DEBUG_SIndexedThirdLevelSearch*	apcIndexedThirdLevelSearch[2];
};


class CIndexedData;
class CIndexedHuge
{
protected:
	CDurableFile*				mpcFile;
	CIndexedData*				mpcIndexedData;  //When indices to cached objects are evicted the object must be evicted also.
	CArrayIndexedSearchPtr		mapFirstLevel;
	void*						mpvSecondLevel;
	void*						mpvThirdLevel;

	int							miSecondLevelChunkWidth;  //Where 'width' is how many descriptor chunk pointers there are per first level.
	int							miThirdLevelChunkWidth;  //Number of CIndexDescriptor's per chunk.
	
	int							miNumSecondLevelChunks;
	int							miNumThirdLevelChunks;

	OIndex						miLastOi;
	BOOL						mbSecondLevelCacheFull;
	SIndexedThirdLevelSearch*	mpsFirstThirdLevelChunk;
	SIndexedThirdLevelSearch*	mpsLastThirdLevelChunk;
	int							miLastSecondLevelChunk;

	BOOL						mbDirtyTesting;

	int							miDiskReads;
	int							miDiskWrites;

public:
	void 						Init(CDurableFile* pcFile, BOOL bDirtyTesting, CIndexedData* pcIndexedData, int iSecondLevelWidth, int iThirdLevelWidth, int iNumSecondLevelChunks, int iNumThirdLevelChunks);
	void 						Kill(void);

	BOOL 						Get(CIndexDescriptor* pcDescriptor, OIndex oi);
	BOOL 						Set(CIndexDescriptor* pcDescriptor);
	BOOL						Set(CIndexDescriptor* pacDescriptors, int iNumDescriptors);
	BOOL						Remove(OIndex oi);

	OIndex						Length(void);

	void						Load(void);
	void						Save(void);

	int							NumPossibleInMemoryIndexDescriptors(void);
	int							GetSecondLevelCacheByteSize(void);
	int							GetThirdLevelCacheByteSize(void);
	BOOL 						ChangeStrategy(int iFirstLevelGrouping, int iSecondLevelWidth, int iNumSecondLevelChunks, int iNumThirdeLevelChunks);
	void						UpdateFile(void);
	void						DumpThirdLevelCache(void);

protected:
	BOOL						PadFile(filePos iLength, filePos iOffset);
	void						ClearCounters(void);

	CIndexDescriptor*			PrivateGetDescriptor(OIndex oi);
	SIndexedSecondLevelSearch*	GetSecondLevelSearch(OIndex oi);
	int							IncrementSecondLevelNumber(int iInput);
	SIndexedSecondLevelSearch*	LoadSecondLevelChunk(int iFirstLevelIndex);
	SIndexedThirdLevelSearch*	LoadThirdLevelChunk(SIndexedSecondLevelSearch* pSIndexedSecondLevelSearch, int iSecondLevelIndex);
	void						SetSecondLevelChunk(SIndexedSecondLevelSearch* psIndexedSecondLevelSearch, int iSecondLevelIndex);
	void						EvictSecondLevelChunk(SIndexedSecondLevelSearch* pSIndexedSecondLevelSearch);
	void						EvictThirdLevelChunk(SIndexedThirdLevelSearch* psIndexedThirdLevelSearch);
	SIndexedThirdLevelSearch*	FindUnallocatedThirdLevelChunk(void);
	SIndexedThirdLevelSearch*	GetCachedThirdLevelChunk(int iIndex);
	CIndexDescriptor*			GetCachedDescriptor(SIndexedThirdLevelSearch* psIndexedThirdLevelSearch, int iIndex);
	SIndexedThirdLevelSearch**	GetIndexedThirdLevelChunk(SIndexedSecondLevelSearch* pSIndexedSecondLevelSearch, int iIndex);
	OIndex						GetThirdLevelChunkOI(SIndexedThirdLevelSearch* psIndexedThirdLevelSearch);
	void						SaveThirdLevelChunk(SIndexedThirdLevelSearch* psIndexedThirdLevelSearch);
	void						EvictCachedObjects(SIndexedThirdLevelSearch* psIndexedThirdLevelSearch);
};


#endif // __INDEXED_HUGE_H__

