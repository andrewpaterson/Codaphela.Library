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
#include "InputSourceDesc.h"
#include "InputVirtualDevice.h"
#include "InputVirtualDeviceCommonality.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDeviceCommonality::Init(CInputVirtualDevice* pcVirtualDevice)
{
	mpcVirtualDevice = pcVirtualDevice;
	mapcSharedDeviceSources.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDeviceCommonality::Kill(void)
{
	mapcSharedDeviceSources.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDeviceCommonality::Clear(void)
{
	mapcSharedDeviceSources.ReInit();
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareInputVirtualDevice(const void* arg1, const void* arg2)
{
	CInputVirtualDevice*	pcVirtual1;
	CInputVirtualDevice*	pcVirtual2;

	pcVirtual1 = (CInputVirtualDevice*)(*((void**)arg1));
	pcVirtual2 = (CInputVirtualDevice*)(*((void**)arg2));

	return pcVirtual1->GetName()->Compare(pcVirtual2->GetName()->Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDeviceCommonality::Sort(void)
{
	mapcSharedDeviceSources.QuickSort(CompareInputVirtualDevice);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDeviceCommonality::AddIfShared(CInputVirtualDevice* pcOtherVirtualDevice)
{
	SSetIterator				sIter;
	CInputVirtualDeviceSource*	pcVirtualSourceDesc;

	pcVirtualSourceDesc = mpcVirtualDevice->StartHistorySourcesIteration(&sIter);
	while (pcVirtualSourceDesc)
	{
		if (pcOtherVirtualDevice->ContainsHistorySource(pcVirtualSourceDesc->GetSource()))
		{
			mapcSharedDeviceSources.AddIfUnique(&pcOtherVirtualDevice);
			break;
		}
		pcVirtualSourceDesc = mpcVirtualDevice->IterateHistorySources(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDeviceCommonality::ToString(CChars* psz)
{
	size					i;
	CInputVirtualDevice*	pcOtherDesc;
	size					uiNumElements;

	psz->Append(" --- CInputVirtualDeviceCommonality ---\n");
	psz->Append(mpcVirtualDevice->GetName()->Text());
	psz->AppendNewLine();

	psz->Append(" - CInputVirtualDevice* : mapcSharedDeviceSources -\n");
	uiNumElements = mapcSharedDeviceSources.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcOtherDesc = *mapcSharedDeviceSources.Get(i);
		psz->Append("  ");
		psz->Append(pcOtherDesc->GetName()->Text());
		psz->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDeviceCommonality::Disable(void)
{
	CInputVirtualDevice*	pcVirtual;
	size					i;
	size					uiNumElements;

	uiNumElements = mapcSharedDeviceSources.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcVirtual = *mapcSharedDeviceSources.Get(i);
		pcVirtual->Disable();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputVirtualDeviceCommonality::Dump(void)
{
	CChars						sz;

	sz.Init();
	ToString(&sz);
	sz.Dump();
	sz.Kill();
}

