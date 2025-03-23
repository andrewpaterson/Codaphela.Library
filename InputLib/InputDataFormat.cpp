/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela InputLib

Codaphela InputLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela InputLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela InputLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "StandardLib/ChannelPadder.h"
#include "StandardLib/SingleChannelAccessor.h"
#include "InputDataFormat.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDataFormat::Init(void)
{
	mcExternalChannels.Init();
	mcExternalChannels.BeginChange();
	mcExternalChannels.SetData((void*)1);
	mcChannelNames.Init();
	mbCommon = false;
	mszCommonName._Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDataFormat::Init(char* szCommonName)
{
	mcExternalChannels.Init();
	mcExternalChannels.BeginChange();
	mcExternalChannels.SetData((void*)1);
	mcChannelNames.Init();
	mbCommon = true;
	mszCommonName.Init(szCommonName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDataFormat::Kill(void)
{
	mszCommonName.Kill();
	mcExternalChannels.Kill();
	mcChannelNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDataFormat::Add(EPrimitiveType eType, char* szName)
{
	mcExternalChannels.AddChannel(mcChannelNames.NumElements(), eType);
	mcChannelNames.Add(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDataFormat::Done(void)
{
	CChannelPadder	cPadder;

	mcExternalChannels.SetSize(1);
	mcExternalChannels.EndChange();
	cPadder.Init(&mcExternalChannels);
	cPadder.OptimalPadding();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CInputDataFormat::GetIndex(char* szName)
{
	size		i;
	CChars*		psz;
	size		uiNumElements;

	uiNumElements = mcChannelNames.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		psz = mcChannelNames.Get(i);
		if (psz->Equals(szName))
		{
			return i;
		}
	}
	return MAX_UINT;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChannel* CInputDataFormat::Get(size iIndex)
{
	return mcExternalChannels.GetChannelAtIndex(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CInputDataFormat::GetConvertToFloat(size iIndex, void* pvData)
{
	CSingleChannelAccessor	cSingle;

	mcExternalChannels.SetData(pvData);
	cSingle.Init(&mcExternalChannels);
	return cSingle.GetConvertToFloat(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CInputDataFormat::GetCastToFloat(size iIndex, void* pvData)
{
	CSingleChannelAccessor	cSingle;

	mcExternalChannels.SetData(pvData);
	cSingle.Init(&mcExternalChannels);
	return cSingle.GetCastToFloat(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDataFormat::GetAsNative(void* pvDest, size iIndex, void* pvData)
{
	CSingleChannelAccessor	cSingle;

	mcExternalChannels.SetData(pvData);
	cSingle.Init(&mcExternalChannels);
	cSingle.GetNative(iIndex, pvDest);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDataFormat::IsCommonName(char* szName)
{
	if (mbCommon)
	{
		return mszCommonName.Equals(szName);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDataFormat::IsCommon(void) { return mbCommon; }
char* CInputDataFormat::GetCommonName(void) { return mszCommonName.Text(); }
