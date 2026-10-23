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
#ifndef __TRACE_H__
#define __TRACE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "Port.h"


//Logicim's Trace: an analog wire.  Its voltage is the average of what the ports connected to it drive it with, NaN
//when nothing drives it.  When it stops being driven its ports are told, see NotifyUndriven.  It doesn't own its ports.  Every trace gets the next id unless it is given one.
class CTrace
{
protected:
	CArrayPortPtr	mapcConnectedPorts;
	int64			miID;

public:
	void			Init(void);
	void			Init(int64 iID);
	void			Kill(void);

	float32			GetVoltage(int64 iTime);
	float32			GetShortVoltage(int64 iTime);
	float32			GetMinimumVoltage(int64 iTime);
	float32			GetMaximumVoltage(int64 iTime);
	void			AppendVoltage(CChars* psz, int64 iTime);

	void			Connect(CPort* pcPort);
	void			NotifyUndriven(CTimeline* pcTimeline, CPort* pcExcept);
	void			Disconnect(CPort* pcPort);
	size			NumConnectedPorts(void);
	CPort*			GetConnectedPort(size uiIndex);
	int64			GetID(void);

	static int64	GetID(CTrace* pcTrace);
	static void		ResetNextID(void);
};


typedef CArrayTemplatePtr<CTrace>	CArrayTracePtr;


#endif // __TRACE_H__

