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
#include "W65MetaBus16.h"
#include "W65MetaBus32.h"
#include "W65MetaTrace.h"


#define NUM_65C64_EDGE_INTERRUPTS	4


//The signals the simulator uses.  These are not the spreadsheet's pinout.
class CTimeline;
class C65C64Pins
{
protected:
	CW65MetaBus32*	mpcAddress;
	CW65MetaBus16*	mpcData;
	CW65MetaTrace*	mpcPHI2;
	CW65MetaTrace*	mpcRESB;
	CW65MetaTrace*	mpcRWB;
	CW65MetaTrace*	mpcLowerByte;
	CW65MetaTrace*	mpcUpperByte;
	CW65MetaTrace*	mpcOPV;
	CW65MetaTrace*	mpcVPA;
	CW65MetaTrace*	mpcVDA;
	CW65MetaTrace*	mpcVPB;
	CW65MetaTrace*	mapcEIRQB[NUM_65C64_EDGE_INTERRUPTS];
	CW65MetaTrace*	mpcLIRQB;

public:
	void			Init(CW65MetaBus32* pcAddress, CW65MetaBus16* pcData, CW65MetaTrace* pcPHI2, CW65MetaTrace* pcRESB, CW65MetaTrace* pcRWB);
	void			SetByteTraces(CW65MetaTrace* pcLowerByte, CW65MetaTrace* pcUpperByte);
	void			SetValidTraces(CW65MetaTrace* pcOPV, CW65MetaTrace* pcVPA, CW65MetaTrace* pcVDA, CW65MetaTrace* pcVPB);
	void			SetInterruptTraces(CW65MetaTrace* pcEIRQB0, CW65MetaTrace* pcEIRQB1, CW65MetaTrace* pcEIRQB2, CW65MetaTrace* pcEIRQB3, CW65MetaTrace* pcLIRQB);
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

