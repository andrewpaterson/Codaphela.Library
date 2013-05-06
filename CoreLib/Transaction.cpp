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
#include "Transaction.h"
#include "TransactionController.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/FastMemcpy.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransaction::Init(CTransactionController* pcController, BOOL bMicro)
{
	mpcController = pcController;
	muiTimeStamp = 0;
	mbFailed = FALSE;
	mbFailEarly = FALSE;
	mcSubTransactions.Init();
	mpcSubTransaction = NULL;
	mbMicro = bMicro;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransaction::KillSubTransactions(void)
{
	CSubTransaction*	pcSubTransaction;

	pcSubTransaction = mcSubTransactions.GetTail();
	while (pcSubTransaction)
	{
		pcSubTransaction->Kill();
		pcSubTransaction = mcSubTransactions.GetPrev(pcSubTransaction);
	}

	mcSubTransactions.Kill();  //It is safe to call kill many times.
	mpcSubTransaction = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransaction::Kill(void)
{
	KillSubTransactions();
	mpcController = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransaction::Begin(unsigned int uiTimeStamp)
{
	BeginSubTransaction();

	mbFailed = FALSE;
	muiTimeStamp = uiTimeStamp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransaction::End(void)
{
	KillSubTransactions();

	muiTimeStamp = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransaction::FailOnTimeStamp(unsigned int uiTimeStamp)
{
	if (muiTimeStamp < uiTimeStamp)
	{
		mbFailed = TRUE;
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransaction::BeginSubTransaction(void)
{
	int		iSubTransactionNum;

	iSubTransactionNum = mcSubTransactions.NumElements();
	mpcSubTransaction = mcSubTransactions.InsertAfterTail();

	mpcSubTransaction->Init(this, iSubTransactionNum);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransaction::EndSubTransaction(BOOL bCommit)
{
	BOOL	bResult;

	if (bCommit)
	{
		bResult = CommitSubTransaction();
	}
	else
	{
		bResult = TRUE;
	}

	if (mpcSubTransaction)
	{
		mpcSubTransaction->Kill();
		mcSubTransactions.RemoveTail();
		mpcSubTransaction = mcSubTransactions.GetTail();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransaction::CommitSubTransaction(void)
{
	CSubTransaction*	pcHead;
	CSubTransaction*	pcParent;
	int					iNumDatas;
	int					i;
	SIndexedMemory*		psIndexedMemory;
	void*				pvData;
	BOOL				bResult;

	pcHead = mcSubTransactions.GetHead();
	if ((mpcSubTransaction == NULL) || (mpcSubTransaction == pcHead))
	{
		//Can't commit the final sub transaction.  Only the transaction controller can do that.
		return FALSE;
	}
	else
	{
		pcParent = mcSubTransactions.GetPrev(mpcSubTransaction);
		iNumDatas = mpcSubTransaction->NumDatas();

		for (i = 0; i < iNumDatas; i++)
		{
			psIndexedMemory = mpcSubTransaction->GetIndexedMemory(i);

			if (psIndexedMemory->uiSize == 0)
			{
				bResult = pcParent->Remove(psIndexedMemory->oi);
				//You don't check to see if it can be removed from the parent 
				//because it may have never existed in the parent but only in the child.
				//(And removing something that doesn't exist fails).
			}
			else
			{
				pvData = mpcSubTransaction->GetData(psIndexedMemory);
				bResult = pcParent->SetOrAdd(psIndexedMemory->oi, pvData, psIndexedMemory->uiSize);
				if (!bResult)
				{
					return FALSE;
				}
			}
		}
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransaction::Add(OIndex oi, void* pvData, unsigned int uiDataSize)
{
	if (mpcSubTransaction)
	{
		return mpcSubTransaction->Add(oi, pvData, uiDataSize);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransaction::Set(OIndex oi, void* pvData, unsigned int uiDataSize)
{
	if (mpcSubTransaction)
	{
		return mpcSubTransaction->Set(oi, pvData, uiDataSize);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransaction::Get(OIndex oi, void* pvData)
{
	if (mpcSubTransaction)
	{
		return mpcSubTransaction->Get(oi, pvData);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransaction::Remove(OIndex oi)
{
	if (mpcSubTransaction)
	{
		return mpcSubTransaction->Remove(oi);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CTransaction::GetData(SIndexedMemory* psIndexedMemory)
{
	return mpcSubTransaction->GetData(psIndexedMemory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIndexedMemory* CTransaction::GetIndexedMemory(int iIndexedDataIndex)
{
	return mpcSubTransaction->GetIndexedMemory(iIndexedDataIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTransaction::NumDatas(void)
{
	return mpcSubTransaction->NumDatas();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CTransaction::GetDirectory(void)
{
	 return mpcController->GetDirectory();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransaction::Dump(void)
{
	mpcSubTransaction->Dump();
}


