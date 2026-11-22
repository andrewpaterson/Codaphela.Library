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
#ifndef __I_R_OPTIMISER_IDIOMS_H__
#define __I_R_OPTIMISER_IDIOMS_H__
#include "IRPasses.h"


//The library's functions a loop becomes a call of (gasIRIdiomLibrary has each one's names and what it takes).
enum EIRIdiom
{
	IRI_Memset,
	IRI_Memmove,
	IRI_Memcmp,
	IRI_Strlen,
	IRI_Memchr,
	IRI_Strchr,
	IRI_Memcpy,			//No loop becomes it (a copy is a memmove): it is here so that its own loop stays a loop.
	IRI_NumIdioms
};


//The shape of a loop the idioms look at (MatchIdiomLoop): one block that goes round to itself, or a head and a
//latch that each leave the loop for a block of their own.
struct SIRIdiomLoop
{
	CIRBlock*	pcHead;
	CIRBlock*	pcLatch;		//The head, in a loop of one block.
	CIRBlock*	pcHeadExit;		//Where the head leaves the loop for, NULL in a loop of one block.
	CIRBlock*	pcExit;			//Where the latch leaves it for.
};


//The loops that are library calls: memset, memmove, strlen, memcmp, memchr and strchr.
class CIRIdiomPasses : public CIRPasses
{
public:
	bool			ReplaceLoopIdioms(void);

protected:
	bool			MatchIdiomLoop(size uiLoop, size uiBlocks, SIRIdiomLoop* psLoop);
	bool			MatchIdiomCounter(size uiLoop, CIRBlock* pcHead, CIRBlock* pcLatch, SIRIdiomCounter* psCounter);
	bool			MatchIdiomStep(size uiLoop, CIRBlock* pcHead, CIRBlock* pcLatch, SIRReference sValue, SIRIdiomCounter* psCounter);
	bool			MatchIdiomAddress(size uiLoop, CIRBlock* pcHead, CIRBlock* pcLatch, SIRReference sAddress, SIRIdiomCounter* psCounter, int64 lliBytes, SIRIdiomAddress* psAddress);
	bool			OnlyUsedInLoop(size uiLoop, CArrayIRReferences* pasAllowed = NULL);
	bool			ReplaceStrlen(size uiLoop);
	bool			ReplaceMemcmp(size uiLoop);
	bool			ReplaceMemchr(size uiLoop);
	bool			ReplaceStrchr(size uiLoop);
	bool			IsLoopValue(size uiLoop, SIRReference sValue);
	bool			FoundValues(size uiLoop, CIRBlock* pcFrom, CIRBlock* pcFound, CArrayIRReferences* pasUsed);
	void			FoundAt(size uiLoop, CIRBlock* pcBlock, SIRReference sTimes, CIRBlock* pcFrom, CIRBlock* pcFound, CArrayIRReferences* pasUsed);
	bool			MatchLinear(size uiLoop, SIRReference sValue, SIRReference* psStart, int64* plliStep, bool* pbNext);
	SIRReference	IdiomStart(CIRBlock* pcBlock, SIRIdiomAddress* psAddress, SIRIdiomCounter* psCounter);
	SIRReference	IdiomBytes(CIRBlock* pcBlock, SIRIdiomCounter* psCounter, int64 lliBytes);
	bool			FewBytes(SIRIdiomCounter* psCounter, int64 lliBytes);
	SIRReference	LibraryByte(CIRBlock* pcBlock, SIRReference sValue);
	const char*		LibrarySize(CIRBlock* pcBlock, EIRIdiom eIdiom, SIRIdiomCounter* psCounter, int64 lliBytes, SIRReference* psBytes, SIRType* psType);
	bool			OnlyLoads(CIRBlock* pcBlock, int iLoads, SIRInstruction** apsLoads);
	bool			GuardedBy(size uiLoop, uint16 uiOpcode, SIRReference sLeft, SIRReference sRight);
	SIRType			LibraryType(uint8 uiKind, SIRType sSize);
	SIRReference	LibraryCall(CIRBlock* pcBlock, EIRIdiom eIdiom, SIRReference sFirst, SIRReference sSecond = IRNone(), SIRReference sBytes = IRNone(), SIRIdiomCounter* psCounter = NULL, int64 lliBytes = 1);
};


#endif // __I_R_OPTIMISER_IDIOMS_H__

