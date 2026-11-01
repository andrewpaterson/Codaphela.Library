/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela MicroprocessorLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MicroprocessorLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MicroprocessorLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/StringHelper.h"
#include "GroupPins.h"


//////////////////////////////////////////////////////////////////////////
//
// The roles' pins in order, each group's after the last's.  A shared
// role's pins are named without a group number.
//
//////////////////////////////////////////////////////////////////////////
void CGroupPins::Init(size uiGroups, SGroupPinRole* asRoles, size uiNumRoles, CVoltageConfigurationSource* pcVoltageConfiguration)
{
	size					i;
	size					j;
	size					k;
	size					uiGroupCount;
	SGroupPinRoleStore*		psRole;
	CLogicPort*				pcPort;
	CChars					szName;

	PreInit();

	CPins::Init();
	muiGroups = (uiGroups < 1) ? 1 : uiGroups;
	masRoles.Init();
	mapcPorts.Init();
	for (i = 0; i < uiNumRoles; i++)
	{
		psRole = masRoles.Add();
		StrCpySafe(psRole->szName, asRoles[i].szName, GROUP_PIN_ROLE_NAME);
		psRole->uiBits = asRoles[i].uiBits;
		psRole->eType = asRoles[i].eType;
		psRole->bShared = asRoles[i].bShared;
		psRole->uiFirstPort = mapcPorts.NumElements();

		uiGroupCount = psRole->bShared ? 1 : muiGroups;
		for (j = 0; j < uiGroupCount; j++)
		{
			for (k = 0; k < psRole->uiBits; k++)
			{
				szName.Init();
				CalculateGroupPortName(&szName, j, uiGroupCount, psRole->szName, k, psRole->uiBits);
				pcPort = NewMalloc<CLogicPort>();
				pcPort->Init(psRole->eType, this, szName.Text(), pcVoltageConfiguration);
				mapcPorts.Add(pcPort);
				szName.Kill();
			}
		}
	}

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGroupPins::Class(void)
{
	CPins::Class();

	U_Size(muiGroups);
	U_Data(CArrayGroupPinRoleStore, masRoles);
	U_Data(CArrayLogicPortPtr, mapcPorts);
}


//////////////////////////////////////////////////////////////////////////
//
// The ports are freed by CPortHolder.
//
//////////////////////////////////////////////////////////////////////////
void CGroupPins::Free(void)
{
	mapcPorts.Kill();
	masRoles.Kill();
	CPins::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// A shared role's pin is the same whatever the group.
//
//////////////////////////////////////////////////////////////////////////
CLogicPort* CGroupPins::GetPin(size uiRole, size uiGroup, size uiBit)
{
	SGroupPinRoleStore*		psRole;

	psRole = masRoles.SafeGet(uiRole);
	if ((psRole == NULL) || (uiBit >= psRole->uiBits) || (uiGroup >= muiGroups))
	{
		return NULL;
	}
	if (psRole->bShared)
	{
		uiGroup = 0;
	}
	return mapcPorts.GetPtr(psRole->uiFirstPort + uiGroup * psRole->uiBits + uiBit);
}


//////////////////////////////////////////////////////////////////////////
//
// The role, group and bit of the port.  A shared role's port is group 0.
//
//////////////////////////////////////////////////////////////////////////
bool CGroupPins::FindPin(CLogicPort* pcPort, size* puiRole, size* puiGroup, size* puiBit)
{
	size					uiIndex;
	size					i;
	SGroupPinRoleStore*		psRole;
	size					uiOffset;

	uiIndex = mapcPorts.Find(pcPort);
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return false;
	}
	for (i = masRoles.NumElements(); i > 0; i--)
	{
		psRole = masRoles.Get(i - 1);
		if (uiIndex >= psRole->uiFirstPort)
		{
			uiOffset = uiIndex - psRole->uiFirstPort;
			*puiRole = i - 1;
			*puiGroup = uiOffset / psRole->uiBits;
			*puiBit = uiOffset % psRole->uiBits;
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The group's bits of the role, bit 0 the least significant.  A bit that
// isn't high reads low; pbAllDriven is whether every bit was high or low.
//
//////////////////////////////////////////////////////////////////////////
uint32 CGroupPins::ReadBits(CTimeline* pcTimeline, size uiRole, size uiGroup, bool* pbAllDriven)
{
	size			i;
	uint32			uiValue;
	STraceValue		sValue;
	bool			bAllDriven;

	uiValue = 0;
	bAllDriven = true;
	for (i = 0; i < GetBitCount(uiRole); i++)
	{
		sValue = GetPin(uiRole, uiGroup, i)->Read(pcTimeline);
		if (sValue.IsHigh())
		{
			uiValue |= 1 << i;
		}
		else if (!sValue.IsLow())
		{
			bAllDriven = false;
		}
	}
	if (pbAllDriven != NULL)
	{
		*pbAllDriven = bAllDriven;
	}
	return uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGroupPins::WriteBits(CTimeline* pcTimeline, size uiRole, size uiGroup, uint32 uiValue)
{
	size	i;

	for (i = 0; i < GetBitCount(uiRole); i++)
	{
		GetPin(uiRole, uiGroup, i)->WriteBool(pcTimeline, ((uiValue >> i) & 1) != 0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGroupPins::WriteImpedance(CTimeline* pcTimeline, size uiRole, size uiGroup)
{
	size	i;

	for (i = 0; i < GetBitCount(uiRole); i++)
	{
		GetPin(uiRole, uiGroup, i)->WriteImpedance(pcTimeline);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGroupPins::IsLow(CTimeline* pcTimeline, size uiRole, size uiGroup, size uiBit)
{
	return GetPin(uiRole, uiGroup, uiBit)->Read(pcTimeline).IsLow();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGroupPins::IsHigh(CTimeline* pcTimeline, size uiRole, size uiGroup, size uiBit)
{
	return GetPin(uiRole, uiGroup, uiBit)->Read(pcTimeline).IsHigh();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CGroupPins::GetGroupCount(void) { return muiGroups; }
size CGroupPins::GetBitCount(size uiRole) { return masRoles.SafeGet(uiRole) != NULL ? masRoles.Get(uiRole)->uiBits : 0; }

