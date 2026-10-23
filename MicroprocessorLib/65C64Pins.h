/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __65C64_PINS_H__
#define __65C64_PINS_H__
#include "BaseLib/PrimitiveTypes.h"
#include "TraceValue.h"
#include "LogicSignal.h"
#include "LogicBus.h"


#define NUM_65C64_EDGE_INTERRUPTS	4


//The signals the simulator uses.  These are not the spreadsheet's pinout.
class CTimeline;
class C65C64Pins
{
protected:
	CLogicBus*		mpcAddress;
	CLogicBus*		mpcData;
	CLogicSignal*	mpcPHI2;
	CLogicSignal*	mpcRESB;
	CLogicSignal*	mpcRWB;
	CLogicSignal*	mpcLowerByte;
	CLogicSignal*	mpcUpperByte;
	CLogicSignal*	mpcOPV;
	CLogicSignal*	mpcVPA;
	CLogicSignal*	mpcVDA;
	CLogicSignal*	mpcVPB;
	CLogicSignal*	mapcEIRQB[NUM_65C64_EDGE_INTERRUPTS];
	CLogicSignal*	mpcLIRQB;

public:
	void			Init(CLogicBus* pcAddress, CLogicBus* pcData, CLogicSignal* pcPHI2, CLogicSignal* pcRESB, CLogicSignal* pcRWB);
	void			SetByteTraces(CLogicSignal* pcLowerByte, CLogicSignal* pcUpperByte);
	void			SetValidTraces(CLogicSignal* pcOPV, CLogicSignal* pcVPA, CLogicSignal* pcVDA, CLogicSignal* pcVPB);
	void			SetInterruptTraces(CLogicSignal* pcEIRQB0, CLogicSignal* pcEIRQB1, CLogicSignal* pcEIRQB2, CLogicSignal* pcEIRQB3, CLogicSignal* pcLIRQB);
	void			Kill(void);

	//Microprocessor side methods.
	void			WriteAddress(CTimeline* pcTimeline, uint32 uiAddress);
	void			WriteData(CTimeline* pcTimeline, uint16 uiData);
	uint16			ReadData(CTimeline* pcTimeline);
	void			WriteRWB(CTimeline* pcTimeline, bool bValue);
	void			WriteByteLanes(CTimeline* pcTimeline, bool bLower, bool bUpper);
	void			WriteOPV(CTimeline* pcTimeline, bool bValue);
	void			WriteVPA(CTimeline* pcTimeline, bool bValue);
	void			WriteVDA(CTimeline* pcTimeline, bool bValue);
	void			WriteVPB(CTimeline* pcTimeline, bool bValue);
	STraceValue		ReadPhi2(CTimeline* pcTimeline);
	STraceValue		ReadRES(CTimeline* pcTimeline);
	STraceValue		ReadEIRQ(CTimeline* pcTimeline, size uiInterrupt);
	STraceValue		ReadLIRQ(CTimeline* pcTimeline);
};


#endif // __65C64_PINS_H__

