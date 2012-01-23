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
#ifndef __SUB_TRANSACTION_H__
#define __SUB_TRANSACTION_H__
#include "IndexedMemory.h"
#include "TransientIndexedFile.h"



class CTransactionController;
class CTransaction;
class CSubTransaction
{
friend class CTransaction;

protected:
	CTransactionIndexedData*	mpcData;

public:
	CTransaction*				mpcTransaction;
	int							miSubTxNum;

	void			Init(CTransaction* pcTransaction, int iSubTxNum, CTransactionIndexedData* pcData = NULL);
	void			Kill(void);

	BOOL			Add(OIndex OI, void* pvData, unsigned int uiDataSize);
	BOOL			Get(OIndex OI, void* pvData);
	BOOL			Set(OIndex OI, void* pvData, unsigned int uiDataSize);
	BOOL			Remove(OIndex OI);

	void*			GetData(SIndexedMemory* psIndexedMemory);
	SIndexedMemory*	GetIndexedMemory(int iIndexedDataIndex);
	int				NumDatas(void);
	BOOL			FailOnTimeStamp(unsigned int uiTimeStamp);

	void			Dump(void);

protected:
	CTransientIndexedFile*		CreateTransientIndexedFile(char* szDirectory);
	BOOL						GetDetail(OIndex oi, void** ppvData, unsigned int* puiSize);
	BOOL						SetOrAdd(OIndex OI, void* pvData, unsigned int uiDataSize);
};


#endif // __SUB_TRANSACTION_H__

