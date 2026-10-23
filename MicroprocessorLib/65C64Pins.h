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
#include "MetaBus16.h"
#include "MetaBus32.h"
#include "MetaTrace.h"


#define NUM_65C64_EDGE_INTERRUPTS	4


//The signals the simulator uses.  These are not the spreadsheet's pinout.
class CTimeline;
class C65C64Pins
{
protected:
	CMetaBus32*		mpcAddress;
	CMetaBus16*		mpcData;
	CMetaTrace*		mpcPHI2;
	CMetaTrace*		mpcRESB;
	CMetaTrace*		mpcRWB;
	CMetaTrace*		mpcLowerByte;
	CMetaTrace*		mpcUpperByte;
	CMetaTrace*		mpcOPV;
	CMetaTrace*		mpcVPA;
	CMetaTrace*		mpcVDA;
	CMetaTrace*		mpcVPB;
	CMetaTrace*		mapcEIRQB[NUM_65C64_EDGE_INTERRUPTS];
	CMetaTrace*		mpcLIRQB;

public:
	void			Init(CMetaBus32* pcAddress, CMetaBus16* pcData, CMetaTrace* pcPHI2, CMetaTrace* pcRESB, CMetaTrace* pcRWB);
	void			SetByteTraces(CMetaTrace* pcLowerByte, CMetaTrace* pcUpperByte);
	void			SetValidTraces(CMetaTrace* pcOPV, CMetaTrace* pcVPA, CMetaTrace* pcVDA, CMetaTrace* pcVPB);
	void			SetInterruptTraces(CMetaTrace* pcEIRQB0, CMetaTrace* pcEIRQB1, CMetaTrace* pcEIRQB2, CMetaTrace* pcEIRQB3, CMetaTrace* pcLIRQB);
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

