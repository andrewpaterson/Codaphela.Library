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
#include "BaseLib/FastMemcmp.h"
#include "TransactionController.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransactionController::Init(char* szDirectory, unsigned int uiCacheSize, BOOL bDurable)
{
	CIndexedConfig	cConfig;
	
	cConfig.OptimiseForTransactions(szDirectory);
	cConfig.SetDurable(bDurable);
	cConfig.SetObjectCacheSize(uiCacheSize);

	mszDirectory.Init(szDirectory);
	mcIndexedData.Init(&cConfig);
	mcTransactions.Init();
	InitState(bDurable);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransactionController::InitState(BOOL bDurable)
{
	filePos		iRead;
	CFileUtil	cFileUtil;

	mszStateName.Init(mszDirectory);
	mszStateRewrite.Init(mszDirectory);

	cFileUtil.AppendToPath(&mszStateName, "State.DAT");
	cFileUtil.AppendToPath(&mszStateRewrite, "_State.DAT");

	mcStateFile.Init(mcIndexedData.GetDurableFileControl(), mszStateName.Text(), mszStateRewrite.Text());
	mcStateFile.Open();
	iRead = mcStateFile.Read(&msState, sizeof(SControllerState), 1);
	if (iRead != 1)
	{
		msState.muiTimeStamp = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransactionController::Kill(void)
{
	CTransaction*	pcTransaction;

	pcTransaction = mcTransactions.GetHead();
	while (pcTransaction)
	{
		//The transaction is canceled/lost.
		pcTransaction->Kill();
		pcTransaction = mcTransactions.GetNext(pcTransaction);
	}

	if (!mcIndexedData.IsDurable())
	{
		WriteState();
	}

	mcIndexedData.Kill();
	mcStateFile.Close();
	mcStateFile.Kill();

	mszDirectory.Kill();
	mszStateName.Kill();
	mszStateRewrite.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTransaction* CTransactionController::BeginTransaction(BOOL bMicro)
{
	CTransaction*	pcTransaction;

	pcTransaction = mcTransactions.InsertAfterTail();
	pcTransaction->Init(this, bMicro);
	pcTransaction->Begin(msState.muiTimeStamp);
	return pcTransaction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransactionController::EndTransaction(CTransaction* pcTransaction, BOOL bCommit)
{
	BOOL	bResult;

	if (bCommit)
	{
		bResult = CommitTransaction(pcTransaction);
	}
	else
	{
		bResult = FALSE;
	}

	pcTransaction->End();
	pcTransaction->Kill();

	mcTransactions.Remove(pcTransaction);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransactionController::ShouldCommitSucceed(CTransaction* pcTransaction)
{
	int					i;
	int					iNumDatas;
	SIndexedMemory*		psIndexedMemory;
	BOOL				bExists;
	CIndexedDataDescriptor	cIndexedDataDescriptor;

	if (pcTransaction->mbFailed)
	{
		return FALSE;
	}

	iNumDatas = pcTransaction->NumDatas();
	for (i = 0; i < iNumDatas; i++)
	{
		psIndexedMemory = pcTransaction->GetIndexedMemory(i);

		bExists = mcIndexedData.GetDescriptor(psIndexedMemory->oi, &cIndexedDataDescriptor);

		if (bExists)
		{
			if (cIndexedDataDescriptor.GetTimeStamp() > pcTransaction->muiTimeStamp)
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransactionController::CommitTransaction(CTransaction* pcTransaction)
{
	int					i;
	int					iNumDatas;
	SIndexedMemory*		psIndexedMemory;
	void*				pvTransactionData;
	BOOL				bResult;

	bResult = ShouldCommitSucceed(pcTransaction);
	if (!bResult)
	{
		return FALSE;
	}

	msState.muiTimeStamp++;
	mcIndexedData.mcDurableFileControl.Begin();

	iNumDatas = pcTransaction->NumDatas();
	for (i = 0; i < iNumDatas; i++)
	{
		psIndexedMemory = pcTransaction->GetIndexedMemory(i);
		if (psIndexedMemory->IsRemoved())
		{
			bResult = mcIndexedData.Remove(psIndexedMemory->oi);
		}
		else
		{
			pvTransactionData = pcTransaction->GetData(psIndexedMemory);
			mcIndexedData.SetOrAdd(psIndexedMemory->oi, pvTransactionData, psIndexedMemory->uiSize, msState.muiTimeStamp);
		}
	}

	if (mcIndexedData.IsDurable())
	{
		mcIndexedData.mcIndices.UpdateFile();
		WriteState();
	}

	mcIndexedData.mcDurableFileControl.End();

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CTransactionController::NumElements(void)
{
	return mcIndexedData.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransactionController::WriteState(void)
{
	filePos		iWritten;

	iWritten = mcStateFile.Write(&msState, sizeof(SControllerState), 1);
	return iWritten == 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CTransactionController::GetDirectory(void)
{
	return mszDirectory.Text();
}

