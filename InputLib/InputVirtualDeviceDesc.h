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
#ifndef __INPUT_VIRTUAL_DEVICE_DESC_H__
#define __INPUT_VIRTUAL_DEVICE_DESC_H__
#include "StandardLib/SetType.h"
#include "InputVirtualSourceDesc.h"
#include "InputChordCriteriaDesc.h"
#include "InputChordDescs.h"


class CInputDeviceDesc;
class CInputDevice;
class CInputDeviceCopyContext;
class CInputVirtualDeviceDesc : public CUnknown
{
CONSTRUCTABLE(CInputVirtualDeviceDesc);
protected:
	CInputChordDescs			mcChordDescs;
	CChars						mszName;
	bool						mbDeviceAgnostic;
	CSetInputVirtualSourceDesc	mlcInputSourceDescs;

public:
	void 						Init(char* szName, bool bDeviceAgnostic);
	void 						Kill(void);

	bool						AddSource(CInputSourceDesc* pcSourceDesc, int iDescriptionID);
	bool 						AddSource(CInputDeviceDesc* pcDeviceDesc, int iDescriptionID, char* szFriendlyName);
	bool 						AddSources(CInputDevice* pcDevice, char* szFriendlyName, ...);
	CInputVirtualSourceDesc*	GetSource(char* szName, int iDescID);
	CInputVirtualSourceDesc*	GetSource(CInputSourceDesc* pcSourceDesc, int iDescID);
	bool						AddChord(char* szActionName, int iDescID, char* szFriendlyName);
	bool						AddOrderedAction(char* szActionName, int iDescID, char* szFriendlyName, ...);

	CInputVirtualSourceDesc* 	StartInputSourceDescsIteration(SSetIterator* psIter);
	CInputVirtualSourceDesc* 	IterateInputSourceDescs(SSetIterator* psIter);
	bool						IsDeviceAgnostic(void);
	char*						GetName(void);
	CInputChordDescs*			GetChordDescs(void);
};


typedef CSetType<CInputVirtualDeviceDesc>	CSetInputVirtualDeviceDesc;


#endif // __INPUT_VIRTUAL_DEVICE_DESC_H__

