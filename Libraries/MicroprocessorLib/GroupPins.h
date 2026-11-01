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
#ifndef __GROUP_PINS_H__
#define __GROUP_PINS_H__
#include "BaseLib/ArrayTemplate.h"
#include "Pins.h"
#include "LogicPort.h"


#define GROUP_PIN_ROLE_NAME		16


//What one kind of a chip's pins does: its name ("D"), how many bits of it each group has (8 for a 74LVC574's data)
//and their port type.  A shared role has the same pins for every group (a 74LVC157's select).
struct SGroupPinRole
{
	const char*		szName;
	size			uiBits;
	EPortType		eType;
	bool			bShared;
};


struct SGroupPinRoleStore
{
	char			szName[GROUP_PIN_ROLE_NAME];
	size			uiBits;
	EPortType		eType;
	bool			bShared;
	size			uiFirstPort;
};


typedef CArrayTemplate<SGroupPinRoleStore>	CArrayGroupPinRoleStore;


//The pins of a chip of groups of bits, as most 74 series parts are: a 74LVC16373 is two groups of an 8 bit latch, a
//74LVC244 two of a 4 bit buffer.  Each role gives a pin for each bit of each group (see CPins::CalculateGroupPortName),
//so a 74LVC16373's data are "1D0" to "1D7" and "2D0" to "2D7", and a shared role one for each bit of it alone.
class CGroupPins : public CPins
{
CONSTRUCTABLE(CGroupPins);
protected:
	size						muiGroups;
	CArrayGroupPinRoleStore		masRoles;
	CArrayLogicPortPtr			mapcPorts;

public:
			void				Init(size uiGroups, SGroupPinRole* asRoles, size uiNumRoles, CVoltageConfigurationSource* pcVoltageConfiguration);
			void				Class(void) override;
			void				Free(void) override;

			size				GetGroupCount(void);
			size				GetBitCount(size uiRole);
			CLogicPort*			GetPin(size uiRole, size uiGroup, size uiBit = 0);
			bool				FindPin(CLogicPort* pcPort, size* puiRole, size* puiGroup, size* puiBit);

			uint32				ReadBits(CTimeline* pcTimeline, size uiRole, size uiGroup, bool* pbAllDriven);
			void				WriteBits(CTimeline* pcTimeline, size uiRole, size uiGroup, uint32 uiValue);
			void				WriteImpedance(CTimeline* pcTimeline, size uiRole, size uiGroup);
			bool				IsLow(CTimeline* pcTimeline, size uiRole, size uiGroup, size uiBit = 0);
			bool				IsHigh(CTimeline* pcTimeline, size uiRole, size uiGroup, size uiBit = 0);
};


#endif // __GROUP_PINS_H__

