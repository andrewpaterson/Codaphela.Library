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
#include "BaseLib/Logger.h"
#include "LogicPort.h"
#include "LogicPortBus.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogicPortBus::Init(void)
{
	muiWidth = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogicPortBus::Kill(void)
{
	muiWidth = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the next most significant bit.
//
//////////////////////////////////////////////////////////////////////////
bool CLogicPortBus::Add(CLogicPort* pcPort)
{
	if (muiWidth >= LOGIC_PORT_BUS_MAX_WIDTH)
	{
		gcLogger.Error2(__METHOD__, " A logic port bus has at most 32 bits.", NULL);
		return false;
	}
	mapcPorts[muiWidth] = pcPort;
	muiWidth++;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogicPortBus::Write(CTimeline* pcTimeline, uint32 uiValue)
{
	size	i;

	for (i = 0; i < muiWidth; i++)
	{
		mapcPorts[i]->WriteBool(pcTimeline, ((uiValue >> i) & 1) != 0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogicPortBus::WriteImpedance(CTimeline* pcTimeline)
{
	size	i;

	for (i = 0; i < muiWidth; i++)
	{
		mapcPorts[i]->WriteImpedance(pcTimeline);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CLogicPortBus::Read(CTimeline* pcTimeline)
{
	size	i;
	uint32	uiValue;

	uiValue = 0;
	for (i = 0; i < muiWidth; i++)
	{
		if (mapcPorts[i]->Read(pcTimeline).IsHigh())
		{
			uiValue |= (uint32)1 << i;
		}
	}
	return uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL past the width.
//
//////////////////////////////////////////////////////////////////////////
CLogicPort* CLogicPortBus::GetPort(size uiBit)
{
	if (uiBit < muiWidth)
	{
		return mapcPorts[uiBit];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CLogicPortBus::GetWidth(void)
{
	return muiWidth;
}

