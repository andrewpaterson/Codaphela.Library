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
#include "NamedIndexedData.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexedData::Init(CIndexedConfig* pcConfig)
{
	mcData.Init(pcConfig);
	mcNames.Init(mcData.GetDurableFileControl(), 10 MB, 256);
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
	ReturnOnFalse(mcNames.Add(oi, szName));
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
BOOL CNamedIndexedData::SetOrAdd(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp)
{
	return mcData.SetOrAdd(oi, pvData, uiDataSize, uiTimeStamp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::SetOrAdd(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp)
{
	ReturnOnFalse(mcNames.Add(oi, szName, FALSE));
	return mcData.SetOrAdd(oi, pvData, uiDataSize, uiTimeStamp);
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
unsigned int CNamedIndexedData::Flags(OIndex oi)
{
	return mcData.Flags(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CNamedIndexedData::GetIndex(CChars* szName)
{
	return mcNames.GetIndex(szName);
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
BOOL CNamedIndexedData::Remove(CChars* szName)
{
	OIndex oi;

	oi = mcNames.GetIndex(szName);
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
BOOL CNamedIndexedData::Flush(void)
{
	ReturnOnFalse(mcNames.Flush());
	return mcData.Flush();
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
OIndex CNamedIndexedData::NumObjects(void)
{
	return mcData.NumElements();	
}

