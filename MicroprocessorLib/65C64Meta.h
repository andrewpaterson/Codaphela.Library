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
#include "65Timeline.h"


class C65C64Meta;
typedef void(*C65C64MetaTick)(C65C64Meta*, void*);


//Owns a 65C64, its pins and the traces they connect to.  Calls TickHigh while PHI2 is
//high (memory responds then) and TickLow while it is low.
class C65C64Meta
{
protected:
	C65Timeline		mcTimeline;
	C65C64			mcMPU;
	C65C64Pins		mcPins;

	C65MetaBus32	mcAddress;
	C65MetaBus16	mcData;
	C65MetaTrace	mcPHI2;
	C65MetaTrace	mcRESB;
	C65MetaTrace	mcRWB;
	C65MetaTrace	mcLowerByte;
	C65MetaTrace	mcUpperByte;
	C65MetaTrace	mcOPV;
	C65MetaTrace	mcVPA;
	C65MetaTrace	mcVDA;
	C65MetaTrace	mcVPB;
	C65MetaTrace	macEIRQB[NUM_65C64_EDGE_INTERRUPTS];
	C65MetaTrace	mcLIRQB;

	C65C64MetaTick	mfTickHigh;
	C65C64MetaTick	mfTickLow;
	void*			mpvContext;

public:
	void			Init(C65C64MetaTick fTickHigh, C65C64MetaTick fTickLow, void* pvContext);
	void			Kill(void);

	C65C64*			GetMPU(void);
	C65MetaBus32*	GetAddress(void);
	C65MetaBus16*	GetData(void);
	C65MetaTrace*	GetPHI2(void);
	C65MetaTrace*	GetRESB(void);
	C65MetaTrace*	GetRWB(void);
	C65MetaTrace*	GetLowerByte(void);
	C65MetaTrace*	GetUpperByte(void);
	C65MetaTrace*	GetOPV(void);
	C65MetaTrace*	GetVPA(void);
	C65MetaTrace*	GetVDA(void);
	C65MetaTrace*	GetVPB(void);
	C65MetaTrace*	GetEIRQB(size uiInterrupt);
	C65MetaTrace*	GetLIRQB(void);

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

