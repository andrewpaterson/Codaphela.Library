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
#ifndef __TRANSIENT_INDEXED_FILE_H__
#define __TRANSIENT_INDEXED_FILE_H__
#include "BaseLib/MemoryCache.h"
#include "TransientIndexedFileDescriptor.h"
#include "ArrayTransientIndexedPointer.h"
#include "TransactionIndexedData.h"


//Remember that in the cache, each data has an SOIndexIndex prepended.
//It is the index back into the mcPointers array.


struct SOIndexIndexCacheDescriptor : public SMemoryCacheDescriptor
{
	SOIndexIndex	sIndex;
};


//This file is a temporary storage for transactions.  
//That is: if a transaction grows larger than memory then this object takes up the slack.
class CTransientIndexedFile : public CTransactionIndexedData
{
public:
	CArrayTransientIndexedFileDescriptor	mcFiles;  //These are the data on disk.  Generally one file per data size.
	CChars									mszDirectory;
	CMemoryCache							mcCache;
	CArrayTransientIndexedPointer			mcPointers;
	BOOL									mbCreatedDirectory;

	void 								Init(char* szDirectory, unsigned int uiCacheSize);
	void 								Kill(void);

	BOOL								Add(OIndex oi, void* pvData, unsigned int uiSize);
	BOOL								Get(OIndex oi, void* pvDest);
	BOOL								Set(OIndex oi, void* pvData, unsigned int uiSize);
	BOOL								Remove(OIndex oi);
	unsigned int						Size(OIndex oi);
	BOOL								AddRemoved(OIndex oi);

	SIndexedMemory*						GetIndexedData(int iIndexedDataIndex);
	void*								GetData(SIndexedMemory* psIndexedMemory);
	int									NumDatas(void);

	BOOL								TestOrder(void);
	void								Dump(void);
	BOOL								GetDetail(OIndex oi, void** ppvData, unsigned int* puiSize);

protected:
	BOOL								PrivateGetDetail(OIndex oi, void** ppvData, unsigned int* puiSize, int* piIndex);
	BOOL								PostAdd(int iIndex, void** ppvData);
	BOOL								Read(STransientIndexedPointer* psPointer, int iPointerIndex);
	BOOL								Write(int iPointerIndex, void* pvData);
	BOOL								WriteNew(STransientIndexedPointer* psPointer, void* pvData);
	BOOL								WriteExisting(STransientIndexedPointer* psPointer, void* pvData);
	void*								PrivateGet(int iIndex);

	BOOL								Allocate(STransientIndexedPointer* psPointer, int iPointerIndex);
	void								RemapCacheOIndexIndices(int iInsertedIndex);

	BOOL								DataFileName(char* szFile1, int iDataSize, int iFileNum);
	CTransientIndexedFileDescriptor* 	GetFileForNewAllocation(int iDataSize);
	CTransientIndexedFileDescriptor* 	GetFile(int iDataSize, int iFileNum);
	CTransientIndexedFileDescriptor* 	GetFile(int iFileIndex);

	BOOL								PrivateAdd(OIndex oi, unsigned int uiSize, void** ppvData, int iIndex);
};


#endif // __TRANSIENT_INDEXED_FILE_H__

