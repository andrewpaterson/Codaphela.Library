/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "BaseLib/Logger.h"
#include "StandardLib/Action.h"
#include "InputChordReader.h"
#include "InputChordDescs.h"
#include "InputCategory.h"
#include "InputDeviceDesc.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordReader::Init(CMarkupTag* pcParentTag, CInputChordDescs* pcInputChordDescs, char* szActionName, CInputDeviceDesc* pcDeviceDesc)
{
	mpcInputChordDescs = pcInputChordDescs;
	mpcParentTag = pcParentTag;
	mpcDeviceDesc = pcDeviceDesc;
	mpcReadChordDesc = NULL;
	mszActionName.Init(szActionName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordReader::Kill(void)
{
	mszActionName.Kill();
	mpcReadChordDesc = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputChordReader::Read(void)
{
	CMarkupTag*			pcActiveTag;
	CMarkupTag*			pcInactiveTag;
	CMarkupTag*			pcGroupTag;
	CMarkupTag*			pcOrderedTag;
	int					iCount;
	CInputChordDesc*	pcChordDesc;
	BOOL				bResult;

	iCount = 0;
	
	pcActiveTag = mpcParentTag->GetTag("Active");
	if (pcActiveTag)
	{
		iCount++;
	}

	pcInactiveTag = mpcParentTag->GetTag("Inactive");
	if (pcInactiveTag)
	{
		iCount++;
	}

	pcGroupTag = mpcParentTag->GetTag("Group");
	if (pcGroupTag)
	{
		iCount++;
	}

	pcOrderedTag = mpcParentTag->GetTag("Ordered");
	if (pcOrderedTag)
	{
		iCount++;
	}

	if (iCount == 0)
	{
		return TRUE;
	}
	else if (iCount == 1)
	{
		mpcReadChordDesc = mpcInputChordDescs->AddChordDesc(mszActionName.Text());
		pcChordDesc = mpcReadChordDesc;
		if (pcActiveTag)
		{
			bResult = ReadActive(pcActiveTag, mpcReadChordDesc, TRUE);
			return bResult;
		}
		else if (pcInactiveTag)
		{
			bResult = ReadActive(pcInactiveTag, mpcReadChordDesc, FALSE);
			return bResult;
		}
		else if (pcGroupTag)
		{
			pcChordDesc->Kill();
			gcLogger.Error("Chord group tag reader not implmented");
			return FALSE;
		}
		else if (pcOrderedTag)
		{
			pcChordDesc->Kill();
			gcLogger.Error("Chord ordered tag reader not implmented");
			return FALSE;
		}
	}
	else
	{
		gcLogger.Error("More than one chord tags found");
		return FALSE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputChordReader::ReadActive(CMarkupTag* pcTag, CInputChordDesc* pcChordDesc, BOOL bActive)
{
	char* 						szGeneric;
	char* 						szFriendly;
	CInputSourceDesc*			pcSource;
	CInputCategoryGeneric*		pcGeneric;

	szGeneric = pcTag->GetAttribute("Generic");
	szFriendly = pcTag->GetAttribute("Friendly");

	if (!szGeneric && !szFriendly)
	{
		gcLogger.Error("No evaluator attribute");
		return FALSE;
	}

	if (szGeneric && szFriendly)
	{
		gcLogger.Error("More than one evaluator attribute");
		return FALSE;
	}

	if (szGeneric)
	{
		pcGeneric = mpcDeviceDesc->GetCategory()->GetGeneric(szGeneric);
		if (!pcGeneric)
		{
			gcLogger.Error2("Generic action [", szGeneric, "] not found for category [", mpcDeviceDesc->GetCategory()->GetCategoryName(), "]", NULL);
			return FALSE;
		}

		if (bActive)
		{
			pcChordDesc->AsActive(pcGeneric);
		}
		else
		{
			pcChordDesc->AsInactive(pcGeneric);
		}
		return TRUE;
	}
	else if (szFriendly)
	{
		pcSource = mpcDeviceDesc->GetSource(szFriendly);
		if (!pcSource)
		{
			gcLogger.Error2("Could not find a source named '", szFriendly, "' on device '", mpcDeviceDesc->GetFriendlyName(), "'", NULL);
			return FALSE;
		}

		if (bActive)
		{
			pcChordDesc->AsActive(pcSource, -1);
		}
		else
		{
			pcChordDesc->AsInactive(pcSource, -1);
		}
		return TRUE;
	}

	return FALSE;
}

