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
#ifndef __PORT_H__
#define __PORT_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "PortType.h"


//Logicim's Port: a connection from a CPortHolder, usually a chip's CPins, to an analog CTrace.  A port is made with
//NewMalloc and its Init adds it to its holder, which owns it from then on and kills and frees it.  Killing a port
//disconnects it from its trace, so ports are killed before their traces, see CCircuit.
//
//Connect builds a circuit before the simulation starts.  ConnectLive wires a port while it runs, as Logicim's editor
//does: the port sees its trace's level, and if connecting it drives an undriven trace the other ports on the trace see
//the new level too.  Disconnect, while it runs, is the reverse: if the trace was driven the port sees undriven, and if
//disconnecting it leaves the trace undriven so do the trace's other ports.  Kill disconnects without telling anyone.
//
//Logicim's OutputPortHelper.getPortOutputVoltage is GetVoltageOut here: the voltage the port drives its trace with,
//NaN for ports that don't drive.
class CTimeline;
class CTrace;
class CPortHolder;
class CPort : public CConstructable
{
CONSTRUCTABLE(CPort);
protected:
	EPortType		meType;
	CChars			mszName;
	CTrace*			mpcTrace;
	CPortHolder*	mpcHolder;
	bool			mbExplicit;

public:
			void			Init(EPortType eType, const char* szName, CPortHolder* pcHolder);
	virtual void			Kill(void);

			bool			Connect(CTrace* pcTrace);
			bool			Connect(CTrace* pcTrace, bool bExplicit);
			bool			ConnectLive(CTrace* pcTrace, CTimeline* pcTimeline);
	virtual void			Disconnect(CTimeline* pcTimeline);
	virtual void			TraceConnected(CTimeline* pcTimeline);
	virtual void			TraceUndriven(CTimeline* pcTimeline);
	virtual void			Reset(void);

	virtual bool			IsLogicPort(void);
	virtual bool			IsPowerIn(void);
	virtual float32			GetVoltageOut(int64 iTime);

			char*			GetName(void);
			EPortType		GetPortType(void);
			CTrace*			GetTrace(void);
			int64			GetTraceID(void);
			CPortHolder*	GetHolder(void);
			bool			IsExplicit(void);
	virtual void			AppendDescription(CChars* psz);
};


typedef CArrayTemplatePtr<CPort>	CArrayPortPtr;


#endif // __PORT_H__

