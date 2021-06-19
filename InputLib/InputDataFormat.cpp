/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
	mbCommon = FALSE;
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
	mbCommon = TRUE;
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
int CInputDataFormat::GetIndex(char* szName)
{
	int			i;
	CChars*		psz;

	for (i = 0; i < mcChannelNames.NumElements(); i++)
	{
		psz = mcChannelNames.Get(i);
		if (psz->Equals(szName))
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChannel* CInputDataFormat::Get(int iIndex)
{
	return mcExternalChannels.GetChannelAtIndex(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CInputDataFormat::GetConvertToFloat(int iIndex, void* pvData)
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
float CInputDataFormat::GetCastToFloat(int iIndex, void* pvData)
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
void CInputDataFormat::GetAsNative(void* pvDest, int iIndex, void* pvData)
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
BOOL CInputDataFormat::IsCommonName(char* szName)
{
	if (mbCommon)
	{
		return mszCommonName.Equals(szName);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDataFormat::IsCommon(void) { return mbCommon; }
char* CInputDataFormat::GetCommonName(void) { return mszCommonName.Text(); }
