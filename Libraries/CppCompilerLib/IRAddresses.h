/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __I_R_ADDRESSES_H__
#define __I_R_ADDRESSES_H__
#include "IRPassAnalysis.h"


//What the pointers at a root are based on (CIRAddresses::PointerBase): a pointer loaded from a restrict symbol
//(uiIndex the symbol), a parameter (uiIndex the parameter, bRestrict if it is restrict) or something unknown.
enum EIRPointerBase
{
	IRPB_Unknown,
	IRPB_RestrictSymbol,
	IRPB_Parameter,
};


struct SIRPointerBase
{
	EIRPointerBase	eKind;
	uint32			uiIndex;
	bool			bRestrict;
};


//A value known to be in memory: what a load read or a store wrote at sRoot + lliOffset.
struct SIRAvailable
{
	SIRReference	sRoot;
	int64			lliOffset;
	SIRType			sType;
	SIRReference	sValue;
};


typedef CArrayTemplate<SIRAvailable>	CArrayIRAvailables;


//What an address is an address of: what the passes ask.  An address is a root and a constant offset from it
//(SplitAddress); two may be the same memory or can't be (MayAlias), by what their roots are (the function's own
//frame, IsFrameRoot, or what a pointer is based on, PointerBase).  IsOwnFrame is the rougher question a store is
//asked, through offsets that aren't constants too: is this anywhere in the function's own frame.  The context's
//definitions must be found.
class CIRAddresses : public CIRPassAnalysis
{
public:
	void			SplitAddress(SIRReference sAddress, SIRReference* psRoot, int64* plliOffset);
	bool			MayAlias(SIRAvailable* psAvailable, SIRReference sRoot, int64 lliOffset, int iBytes);
	bool			IsOwnFrame(SIRReference sAddress);

protected:
	bool			IsFrameRoot(SIRReference sRoot);
	void			PointerBase(SIRReference sRoot, SIRPointerBase* psBase);
};


#endif // __I_R_ADDRESSES_H__
