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
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/FastMemcpy.h"
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/ConstructorCall.h"
#include "Transaction.h"
#include "SubTransaction.h"
#include "TransactionController.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSubTransaction::Init(CTransaction* pcTransaction, int iSubTxNum, CTransactionIndexedData* pcData)
{
	char*	szDirectory;

	mpcTransaction = pcTransaction;
	miSubTxNum = iSubTxNum;
	if (pcData == NULL)
	{
		szDirectory = mpcTransaction->GetDirectory();
		pcData = CreateTransientIndexedFile(szDirectory);
	}
	mpcData = pcData;

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSubTransaction::Kill(void)
{
	mpcData->Kill();

	mpcTransaction = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTransientIndexedFile* CSubTransaction::CreateTransientIndexedFile(char* szDirectory)
{
	CTransientIndexedFile*	pcData;
	CChars	sz;

	sz.Init(szDirectory);
	sz.Append(FILE_SEPARATOR[0]);
	sz.Append("Transaction_");
	sz.Append(mpcTransaction->muiTimeStamp);
	sz.Append("_");
	sz.Append(miSubTxNum);

	pcData = Malloc(CTransientIndexedFile);
	pcData->Init(sz.Text(), 1 MB);

	sz.Kill();
	return pcData;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSubTransaction::FailOnTimeStamp(unsigned int uiTimeStamp)
{
	return mpcTransaction->FailOnTimeStamp(uiTimeStamp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSubTransaction::Add(OIndex oi, void* pvData, unsigned int uiDataSize)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL				bExistsInFile;
	BOOL				bExistsInTransaction;
	unsigned int		uiSize;
	void*				pvCurrent;

	bExistsInTransaction = mpcData->GetDetail(oi, &pvCurrent, &uiSize);
	bExistsInFile = mpcTransaction->mpcController->mcIndexedData.GetDescriptor(oi, &cDescriptor);

	if (mpcTransaction->mbFailEarly)
	{
		FailOnTimeStamp(cDescriptor.GetTimeStamp());
	}

	if (bExistsInTransaction)
	{
		if (uiSize == 0)  
		{
			//If the existing data size is zero than it has been remove in transaction
			//and can be added.
			return mpcData->Set(oi, pvData, uiDataSize);
		}
		else
		{
			//Already added, can't add again.
			return FALSE;
		}
	}
	else
	{
		if (bExistsInFile)
		{
			return FALSE;
		}

		return mpcData->Add(oi, pvData, uiDataSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSubTransaction::Get(OIndex oi, void* pvData)
{
	BOOL				bExistsInTransaction;
	void*				pvCache;
	unsigned int		uiSize;
	BOOL				bExistsInFile;
	CIndexedDataDescriptor	cDescriptor;

	bExistsInTransaction = GetDetail(oi, &pvCache, &uiSize);
	if (bExistsInTransaction)
	{
		if (uiSize == 0)
		{
			return FALSE;
		}
		memcpy_fast(pvData, pvCache, uiSize);
		return TRUE;
	}
	else
	{
		bExistsInFile = mpcTransaction->mpcController->mcIndexedData.GetDescriptor(oi, &cDescriptor);
		if (bExistsInFile)
		{
			return mpcTransaction->mpcController->mcIndexedData.GetData(&cDescriptor, pvData);
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSubTransaction::Set(OIndex oi, void* pvData, unsigned int uiDataSize)
{
	BOOL				bExistsInTransaction;
	void*				pvCache;
	unsigned int		uiSize;
	BOOL				bExistsInFile;
	CIndexedDataDescriptor	cDescriptor;

	if (uiDataSize == 0)
	{
		return FALSE;
	}

	bExistsInTransaction = GetDetail(oi, &pvCache, &uiSize);
	bExistsInFile = mpcTransaction->mpcController->mcIndexedData.GetDescriptor(oi, &cDescriptor);

	if (mpcTransaction->mbFailEarly)
	{
		FailOnTimeStamp(cDescriptor.GetTimeStamp());
	}

	if (bExistsInTransaction)
	{
		if (uiSize != 0)
		{
			return mpcData->Set(oi, pvData, uiDataSize);
		}
		else
		{
			return mpcData->Add(oi, pvData, uiDataSize);
		}
	}
	else
	{
		if (bExistsInFile)
		{
			return mpcData->Add(oi, pvData, uiDataSize);
		}
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSubTransaction::GetDetail(OIndex oi, void** ppvData, unsigned int* puiSize)
{
	BOOL				bExistsInTransaction;

	bExistsInTransaction = mpcData->GetDetail(oi, ppvData, puiSize);
	return bExistsInTransaction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSubTransaction::Remove(OIndex oi)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL				bExistsInTransaction;
	BOOL				bExistsInFile;
	void*				pvData;
	unsigned int		uiSize;

	bExistsInTransaction = mpcData->GetDetail(oi, &pvData, &uiSize);

	if ((mpcTransaction->mbFailEarly) || (!bExistsInTransaction))
	{
		bExistsInFile = mpcTransaction->mpcController->mcIndexedData.GetDescriptor(oi, &cDescriptor);
		if (bExistsInFile)
		{
			FailOnTimeStamp(cDescriptor.GetTimeStamp());

			if (bExistsInTransaction)
			{
				if (uiSize != 0)
				{
					return mpcData->Remove(oi);
				}
				return FALSE;
			}
			else
			{
				return mpcData->AddRemoved(oi);
			}
		}
	}

	if (bExistsInTransaction)
	{
		if (pvData != NULL)
		{
			return mpcData->Remove(oi);
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIndexedMemory* CSubTransaction::GetIndexedMemory(int iIndexedDataIndex)
{
	return mpcData->GetIndexedData(iIndexedDataIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CSubTransaction::NumDatas(void)
{
	return mpcData->NumDatas();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CSubTransaction::GetData(SIndexedMemory* psIndexedMemory)
{
	return mpcData->GetData(psIndexedMemory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSubTransaction::Dump(void)
{
	mpcData->Dump();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSubTransaction::SetOrAdd(OIndex oi, void* pvData, unsigned int uiDataSize)
{
	BOOL			bExistsInTransaction;
	void*			pvCache;
	unsigned int	uiSize;

	bExistsInTransaction = GetDetail(oi, &pvCache, &uiSize);
	if (bExistsInTransaction)
	{
		if (uiDataSize != uiSize)
		{
			mpcData->Remove(oi);
			return mpcData->Add(oi, pvData, uiDataSize);
		}
		else
		{
			memcpy_fast(pvCache, pvData, uiSize);
			return TRUE;
		}
	}
	else
	{
		return mpcData->Add(oi, pvData, uiDataSize);
	}
}

