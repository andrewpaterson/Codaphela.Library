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
#ifndef __65C64_META_H__
#define __65C64_META_H__
#include "65C64.h"
#include "Timeline.h"


class C65C64Meta;
typedef void(*C65C64MetaTick)(C65C64Meta*, void*);


//Owns a 65C64, its pins and the traces they connect to.  Calls TickHigh while PHI2 is
//high (memory responds then) and TickLow while it is low.
class C65C64Meta
{
protected:
	CTimeline		mcTimeline;
	C65C64			mcMPU;
	C65C64Pins		mcPins;

	CW65MetaBus32	mcAddress;
	CW65MetaBus16	mcData;
	CW65MetaTrace	mcPHI2;
	CW65MetaTrace	mcRESB;
	CW65MetaTrace	mcRWB;
	CW65MetaTrace	mcLowerByte;
	CW65MetaTrace	mcUpperByte;
	CW65MetaTrace	mcOPV;
	CW65MetaTrace	mcVPA;
	CW65MetaTrace	mcVDA;
	CW65MetaTrace	mcVPB;
	CW65MetaTrace	macEIRQB[NUM_65C64_EDGE_INTERRUPTS];
	CW65MetaTrace	mcLIRQB;

	C65C64MetaTick	mfTickHigh;
	C65C64MetaTick	mfTickLow;
	void*			mpvContext;

public:
	void			Init(C65C64MetaTick fTickHigh, C65C64MetaTick fTickLow, void* pvContext);
	void			Kill(void);

	C65C64*			GetMPU(void);
	CW65MetaBus32*	GetAddress(void);
	CW65MetaBus16*	GetData(void);
	CW65MetaTrace*	GetPHI2(void);
	CW65MetaTrace*	GetRESB(void);
	CW65MetaTrace*	GetRWB(void);
	CW65MetaTrace*	GetLowerByte(void);
	CW65MetaTrace*	GetUpperByte(void);
	CW65MetaTrace*	GetOPV(void);
	CW65MetaTrace*	GetVPA(void);
	CW65MetaTrace*	GetVDA(void);
	CW65MetaTrace*	GetVPB(void);
	CW65MetaTrace*	GetEIRQB(size uiInterrupt);
	CW65MetaTrace*	GetLIRQB(void);

	bool			TickInstruction(void);
	bool			TestTickCycle(void);

	void			Print(CChars* psz);
	void			Dump(void);

protected:
	void			InputTransition(void);
	void			CallTickHigh(void);
	void			CallTickLow(void);
};


#endif // __65C64_META_H__

