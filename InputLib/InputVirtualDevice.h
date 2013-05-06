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
#ifndef __INPUT_VIRTUAL_DEVICE_H__
#define __INPUT_VIRTUAL_DEVICE_H__
#include "StandardLib/SetType.h"
#include "StandardLib/ListenerCall.h"

#include "StandardLib/Actions.h"

#include "InputDeviceValue.h"
#include "InputVirtualDeviceSource.h"
#include "InputVirtualDeviceCommonality.h"
#include "InputDevicePointerArray.h"
#include "InputChords.h"


class CInputDevices;
class CInputDeviceValues;
class CInputVirtualDevice : public CUnknown
{
BASE_FUNCTIONS(CInputVirtualDevice);
protected:
	CChars							mszName;
	CSetInputVirtualDeviceSource	mlcHistorySources;
	CInputVirtualDeviceCommonality	mcCommonality;
	CArrayInputDeviceValuePtr		mapcInputValues;
	CListenerCall					mcListeners;
	CInputChords					mcChords;
	CArrayInputDevicePtr			mapcDevices;
	BOOL							mbEnabled;
	BOOL							mbUnstoppable;

public:
	void 						Init(char* szName);
	void 						Kill(void);

	void 						UpdateEvents(void);
	void 						UpdateEvents(CInputDeviceValues* pcValues);
	void 						CallListeners(void);
	void 						ClearEvents(void);
	void 						SortEvents(void);
	void						ToStringEvents(CChars* psz);

	BOOL 						AddSource(CInputDevice* pcDevice, CInputSourceDesc* pcSource);
	BOOL						AddSource(CInputDevice* pcDevice, char* szFriendlyName);
	BOOL						AddSources(CInputDevice* pcDevice, char* szFriendlyName, ...);
	void 						DoneAddingSources(void);
	void 						ClearSources(void);

	BOOL 						ContainsHistorySource(SInputDeviceValueSource* psSource);
	BOOL						ContainsSource(SInputDeviceValueSource* psSource, CSetInputVirtualDeviceSource* pcSetSources);
	CInputVirtualDeviceSource*	StartHistorySourcesIteration(SSetIterator* psIter);
	CInputVirtualDeviceSource*	IterateHistorySources(SSetIterator* psIter);

	void 						ClearCommonality(void);
	void 						AddCommonalityIfShared(CInputVirtualDevice* pcOtherVirtual);
	void 						SortCommonality(void);

	void						Enable(BOOL bDisableCommon = TRUE);
	void						Disable(void);
	void						Unstoppable(void);

	CInputDevices*				GetInputDevices(void);
	CInputDevice*				GetDefaultDevice(void);
	CInputVirtualDeviceSource*	GetSource(char* szFriendlyName);
	CInputChords*				GetChords(void);

	void						AddActiveAction(CAction* pcAction, char* szFriendlyName);
	void						AddInactiveAction(CAction* pcAction, char* szFriendlyName);
	void						AddActiveAction(CAction* pcAction, CInputCategoryGeneric* pcGeneric);
	void						AddInactiveAction(CAction* pcAction, CInputCategoryGeneric* pcGeneric);
	void						AddActiveAction(CAction* pcAction, CInputVirtualDeviceSource* pcSource);
	void						AddInactiveAction(CAction* pcAction, CInputVirtualDeviceSource* pcSource);
	void						AddSingleAction(CAction* pcAction, CInputSourceEvaluator* pcEvaluator, BOOL bActive);
	void						AddOrderedAction(CAction* pcAction, char* szFriendlyName, ...);
	CInputChord*				AddChordAction(CAction* pcAction, CInputChordDesc* pcChordDesc = NULL);
	void						AddRangeAction(CAction* pcAction, char* szFriendlyName);
	void						AddRangeAction(CAction* pcAction, CInputVirtualDeviceSource* pcSource);

	CChars*						GetName(void);
	CListenerCall*				GetListeners(void);

	void						Dump(void);
};


typedef CSetType<CInputVirtualDevice>	CSetInputVirtualDevice;


#endif // __INPUT_VIRTUAL_DEVICE_H__

