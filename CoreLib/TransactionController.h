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
#ifndef __TRANSACTION_CONTROLLER_H__
#define __TRANSACTION_CONTROLLER_H__
#include "BaseLib/LinkedListTemplate.h"
#include "BaseLib/Chars.h"
#include "Transaction.h"
#include "DurableSet.h"
#include "IndexedData.h"


typedef	CLinkedListTemplate<CTransaction>	CListTransactions;


struct SControllerState
{
	unsigned int	muiTimeStamp;
};


class CTransactionController
{
protected:
	CListTransactions			mcTransactions;
	CChars						mszDirectory;
	CDurableFile				mcStateFile;
	CChars						mszStateName;
	CChars						mszStateRewrite;
	SControllerState			msState;

public:
	CIndexedData				mcIndexedData;

	void 			Init(char* szDirectory, unsigned int uiCacheSize, BOOL bDurable);
	void 			Kill(void);
	void			InitState(BOOL bDurable);

	CTransaction*	BeginTransaction(BOOL bMicro = FALSE);
	BOOL			EndTransaction(CTransaction* pcTransaction, BOOL bCommit);
	char*			GetDirectory(void);

	OIndex			NumElements(void);
	
	BOOL			WriteState(void);

protected:
	BOOL	ShouldCommitSucceed(CTransaction* pcTransaction); 
	BOOL	CommitTransaction(CTransaction* pcTransaction);
};


#endif // __TRANSACTION_CONTROLLER_H__

