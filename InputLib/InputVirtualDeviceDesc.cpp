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
#include "InputVirtualDeviceDesc.h"
#include "InputDeviceCopyContext.h"
#include "InputDevice.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDeviceDesc::Init(char* szName, BOOL bDeviceAgnostic)
{
	mlcInputSourceDescs.Init();
	mcChordDescs.Init();
	mszName.Init(szName);
	mbDeviceAgnostic = bDeviceAgnostic;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDeviceDesc::Kill(void)
{
	mszName.Kill();
	mcChordDescs.Kill();
	mlcInputSourceDescs.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputVirtualDeviceDesc::AddSource(CInputSourceDesc* pcSourceDesc, int iDescriptionID)
{
	CInputVirtualSourceDesc*	pcVirtualSourceDesc;

	if (mbDeviceAgnostic)
	{
		pcVirtualSourceDesc = mlcInputSourceDescs.Add();
		pcVirtualSourceDesc->Init(this, pcSourceDesc, -1);
		return TRUE;
	}
	else
	{
		if (iDescriptionID < 0)
		{
			return FALSE;
		}
		pcVirtualSourceDesc = mlcInputSourceDescs.Add();
		pcVirtualSourceDesc->Init(this, pcSourceDesc, iDescriptionID);
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputVirtualDeviceDesc::AddSource(CInputDeviceDesc* pcDeviceDesc, int iDescriptionID, char* szFriendlyName)
{
	CInputSourceDesc*			pcSourceDesc;
	BOOL						bResult;

	pcSourceDesc = pcDeviceDesc->GetSource(szFriendlyName);
	if (pcSourceDesc)
	{
		bResult = AddSource(pcSourceDesc, iDescriptionID);
		return bResult;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputVirtualDeviceDesc::AddSources(CInputDevice* pcDevice, char* szFriendlyName, ...)
{
	va_list				vaMarker;
	char*				pc;
	int					iCount;
	BOOL				bResult;
	CInputDeviceDesc*	pcDeviceDesc;

	iCount = 0;
	pcDeviceDesc = pcDevice->GetDesc();

	pc = szFriendlyName;
	va_start(vaMarker, szFriendlyName);
	while (pc != NULL)
	{
		bResult = AddSource(pcDeviceDesc, pcDevice->GetDescriptionID(), pc);
		if (!bResult)
		{
			va_end(vaMarker);
			return FALSE;
		}

		iCount++;
		pc = va_arg(vaMarker, char*);
	}
	va_end(vaMarker);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputVirtualSourceDesc* CInputVirtualDeviceDesc::GetSource(char* szFriendlyName, int iDescID)
{
	CInputVirtualSourceDesc*	pcSource;
	SSetIterator				sIter;

	if ((!mbDeviceAgnostic) && (iDescID < 0))
	{
		return NULL;
	}

	pcSource = mlcInputSourceDescs.StartIteration(&sIter);
	while (pcSource)
	{
		if ((mbDeviceAgnostic) || (pcSource->GetDescriptionID() == iDescID))
		{
			if (pcSource->GetSourceDesc()->Is(szFriendlyName))
			{
				return pcSource;
			}
		}
		pcSource = mlcInputSourceDescs.Iterate(&sIter);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputVirtualSourceDesc* CInputVirtualDeviceDesc::GetSource(CInputSourceDesc* pcSourceDesc, int iDescID)
{
	CInputVirtualSourceDesc*	pcSource;
	SSetIterator				sIter;

	pcSource = mlcInputSourceDescs.StartIteration(&sIter);
	while (pcSource)
	{
		if ((mbDeviceAgnostic) || (pcSource->GetDescriptionID() == iDescID))
		{
			if (pcSource->GetSourceDesc() == pcSourceDesc)
			{
				return pcSource;
			}
		}
		pcSource = mlcInputSourceDescs.Iterate(&sIter);
	}
	return NULL;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputVirtualDeviceDesc::AddChord(char* szActionName, int iDescID, char* szFriendlyName)
{
	CInputVirtualSourceDesc*	pcSource;
	CInputChordDesc*			pcChord;

	pcSource = GetSource(szFriendlyName, iDescID);
	if (pcSource == NULL)
	{
		return FALSE;
	}

	pcChord = mcChordDescs.AddChordDesc(szActionName);
	pcChord->AsActive(pcSource->GetSourceDesc(), pcSource->GetDescriptionID());
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputVirtualDeviceDesc::AddOrderedAction(char* szActionName, int iDescID, char* szFriendlyName, ...)
{
	va_list						vaMarker;
	char*						pc;
	int							iCount;
	CInputChordDesc*			pcChord;
	CInputVirtualSourceDesc*	pcSource;
	CInputChordCollectiveDesc*	pcOrdered;
	BOOL						bResult;

	pcChord = mcChordDescs.AddChordDesc(szActionName);
	pcOrdered = pcChord->AsOrdered();

	bResult = TRUE;
	iCount = 0;
	pc = szFriendlyName;
	va_start(vaMarker, szFriendlyName);
	while (pc != NULL)
	{
		pcSource = GetSource(pc, iDescID);
		if (pcSource == NULL)
		{
			va_end(vaMarker);
			return FALSE;
		}
		pcOrdered->AddActive(pcSource->GetSourceDesc(), pcSource->GetDescriptionID());
		pcOrdered->AddInactive(pcSource->GetSourceDesc(), pcSource->GetDescriptionID());

		iCount++;
		pc = va_arg(vaMarker, char*);
	}

	pcOrdered->RemoveLast();
	va_end(vaMarker);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputVirtualDeviceDesc::IsDeviceAgnostic(void) { return mbDeviceAgnostic; }
char* CInputVirtualDeviceDesc::GetName(void) { return mszName.Text(); }
CInputVirtualSourceDesc* CInputVirtualDeviceDesc::StartInputSourceDescsIteration(SSetIterator* psIter) { return mlcInputSourceDescs.StartIteration(psIter); }
CInputVirtualSourceDesc* CInputVirtualDeviceDesc::IterateInputSourceDescs(SSetIterator* psIter) { return mlcInputSourceDescs.Iterate(psIter); }
CInputChordDescs* CInputVirtualDeviceDesc::GetChordDescs(void) { return &mcChordDescs; }
