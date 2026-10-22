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
#include "65TraceValue.h"
#include "65MetaBus16.h"
#include "65MetaBus32.h"
#include "65MetaTrace.h"


#define NUM_65C64_EDGE_INTERRUPTS	4


//The signals the simulator uses.  These are not the spreadsheet's pinout.
class C65Timeline;
class C65C64Pins
{
protected:
	C65MetaBus32*	mpcAddress;
	C65MetaBus16*	mpcData;
	C65MetaTrace*	mpcPHI2;
	C65MetaTrace*	mpcRESB;
	C65MetaTrace*	mpcRWB;
	C65MetaTrace*	mpcLowerByte;
	C65MetaTrace*	mpcUpperByte;
	C65MetaTrace*	mpcOPV;
	C65MetaTrace*	mpcVPA;
	C65MetaTrace*	mpcVDA;
	C65MetaTrace*	mpcVPB;
	C65MetaTrace*	mapcEIRQB[NUM_65C64_EDGE_INTERRUPTS];
	C65MetaTrace*	mpcLIRQB;

public:
	void			Init(C65MetaBus32* pcAddress, C65MetaBus16* pcData, C65MetaTrace* pcPHI2, C65MetaTrace* pcRESB, C65MetaTrace* pcRWB);
	void			SetByteTraces(C65MetaTrace* pcLowerByte, C65MetaTrace* pcUpperByte);
	void			SetValidTraces(C65MetaTrace* pcOPV, C65MetaTrace* pcVPA, C65MetaTrace* pcVDA, C65MetaTrace* pcVPB);
	void			SetInterruptTraces(C65MetaTrace* pcEIRQB0, C65MetaTrace* pcEIRQB1, C65MetaTrace* pcEIRQB2, C65MetaTrace* pcEIRQB3, C65MetaTrace* pcLIRQB);
	void			Kill(void);

	//Microprocessor side methods.
	void			WriteAddress(C65Timeline* pcTimeline, uint32 uiAddress);
	void			WriteData(C65Timeline* pcTimeline, uint16 uiData);
	uint16			ReadData(C65Timeline* pcTimeline);
	void			WriteRWB(C65Timeline* pcTimeline, bool bValue);
	void			WriteByteLanes(C65Timeline* pcTimeline, bool bLower, bool bUpper);
	void			WriteOPV(C65Timeline* pcTimeline, bool bValue);
	void			WriteVPA(C65Timeline* pcTimeline, bool bValue);
	void			WriteVDA(C65Timeline* pcTimeline, bool bValue);
	void			WriteVPB(C65Timeline* pcTimeline, bool bValue);
	S65TraceValue	ReadPhi2(C65Timeline* pcTimeline);
	S65TraceValue	ReadRES(C65Timeline* pcTimeline);
	S65TraceValue	ReadEIRQ(C65Timeline* pcTimeline, size uiInterrupt);
	S65TraceValue	ReadLIRQ(C65Timeline* pcTimeline);
};


#endif // __65C64_PINS_H__

