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
#ifndef __TRANSACTION_H__
#define __TRANSACTION_H__
#include "BaseLib/LinkListTemplate.h"
#include "SubTransaction.h"


typedef CLinkListTemplate<CSubTransaction> CLinkListSubTransactions;


class CTransactionController;
class CTransaction
{
public:
	CTransactionController*		mpcController;
	unsigned int				muiTimeStamp;
	BOOL						mbFailEarly;
	BOOL						mbFailed;
	CLinkListSubTransactions	mcSubTransactions;
	CSubTransaction*			mpcSubTransaction;
	BOOL						mbMicro;

	void			Init(CTransactionController* pcController, BOOL bMicro);
	void			Kill(void);
	void			KillSubTransactions(void);

	void			Begin(unsigned int uiTimeStamp);
	void			End(void);

	BOOL			Add(OIndex oi, void* pvData, unsigned int uiDataSize);
	BOOL			Get(OIndex oi, void* pvData);
	BOOL			Set(OIndex oi, void* pvData, unsigned int uiDataSize);
	BOOL			Remove(OIndex oi);

	SIndexedMemory*	GetIndexedMemory(int iIndexedDataIndex);
	void*			GetData(SIndexedMemory* psIndexedMemory);
	int				NumDatas(void);
	BOOL			FailOnTimeStamp(unsigned int uiTimeStamp);

	void			BeginSubTransaction(void);
	BOOL			EndSubTransaction(BOOL bCommit);
	char*			GetDirectory(void);

	void			Dump(void);

protected:
	BOOL			CommitSubTransaction(void);
};


#endif // __TRANSACTION_H__

