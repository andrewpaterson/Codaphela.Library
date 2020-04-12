/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "CoreLib/NamedIndexedData.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexedData::Init(CNamedIndexConfig* pcConfig)
{
	mcData.Init(pcConfig->GetIndexConfig());
	//mcNames.Init(pcConfig->GetNamedConfig());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexedData::Kill(void)
{
	mcNames.Kill();
	mcData.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Add(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp)
{
	return mcData.Add(oi, pvData, uiDataSize, uiTimeStamp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Add(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp)
{
	ReturnOnFalse(mcNames.Put(szName, oi));
	return mcData.Add(oi, pvData, uiDataSize, uiTimeStamp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Set(OIndex oi, void* pvData, unsigned int uiTimeStamp)
{
	return mcData.Set(oi, pvData, uiTimeStamp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Set(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp)
{
	return mcData.Set(oi, pvData, uiDataSize, uiTimeStamp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Put(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp)
{
	return mcData.Put(oi, pvData, uiDataSize, uiTimeStamp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Put(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp)
{
	ReturnOnFalse(mcNames.Put(szName, oi, FALSE));
	return mcData.Put(oi, pvData, uiDataSize, uiTimeStamp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Put(OIndex oi, char* szName, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp)
{
	ReturnOnFalse(mcNames.Put(szName, oi, FALSE));
	return mcData.Put(oi, pvData, uiDataSize, uiTimeStamp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CNamedIndexedData::Size(OIndex oi)
{
	return mcData.Size(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Get(OIndex oi, void* pvData)
{
	return mcData.Get(oi, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Get(OIndex oi, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxSize)
{
	return mcData.Get(oi, puiDataSize, pvData, uiMaxSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Get(char* szName, void* pvData)
{
	OIndex	oi;
	BOOL	bResult;

	oi = mcNames.Get(szName);
	if (oi != INVALID_O_INDEX)
	{
		bResult = mcData.Get(oi, pvData);
		return bResult;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Get(char* szName, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxSize)
{
	OIndex	oi;
	BOOL	bResult;

	oi = mcNames.Get(szName);
	if (oi != INVALID_O_INDEX)
	{
		bResult = mcData.Get(oi, puiDataSize, pvData, uiMaxSize);
		return bResult;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CNamedIndexedData::GetIndex(char* szName)
{
	OIndex	oi;

	oi = mcNames.Get(szName);
	return oi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Contains(OIndex oi)
{
	return mcData.Contains(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Contains(char* szName)
{
	OIndex	oi;

	oi = mcNames.Get(szName);
	if (oi != INVALID_O_INDEX)
	{
		//You might need to get the descriptor and make sure its not NULL.
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Remove(CChars* szName)
{
	OIndex oi;

	oi = mcNames.Get(szName);
	if (oi != INVALID_O_INDEX)
	{
		ReturnOnFalse(mcNames.Remove(szName));
	}
	else
	{
		return FALSE;
	}
	return mcData.Remove(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Remove(OIndex oi)
{
	return mcData.Remove(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Flush(BOOL bClearCache)
{
	BOOL	bResult;

	bResult = mcNames.Flush();
	bResult &= mcData.Flush(bClearCache);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexedData::DurableBegin(void)
{
	mcData.DurableBegin();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexedData::DurableEnd(void)
{
	mcData.DurableEnd();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
long long int CNamedIndexedData::NumIndicies(void)
{
	return mcData.NumElements();	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
long long int CNamedIndexedData::NumNames(void)
{
	return mcNames.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::IsCaching(void)
{
	return mcData.IsCaching();
}

