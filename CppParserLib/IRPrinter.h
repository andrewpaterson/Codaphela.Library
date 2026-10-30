/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __I_R_PRINTER_H__
#define __I_R_PRINTER_H__
#include "IRModule.h"


//Writes a module or a function as IR text (IR.txt Text form).  CIRParser reads it back.  Unnamed temporaries are
//numbered in the order they are defined and unnamed blocks in block order, so printing what was parsed gives the
//same text.
class CIRPrinter
{
protected:
	CIRModule*				mpcModule;
	CChars*					mpszDest;
	CArrayTemplate<uint32>	mauiTemporaryNumbers;	//Per temporary, for the function being printed.
	CArrayTemplate<uint32>	mauiBlockNumbers;
	bool					mbSources;				//A line with the source (IR_SOURCE_MARKER) where it changes (SetSources).

public:
	void	Init(CIRModule* pcModule, CChars* pszDest);
	void	Kill(void);
	void	SetSources(bool bSources);

	void	PrintModule(void);
	void	PrintFunction(CIRFunction* pcFunction);
	void	PrintGlobal(CIRGlobal* pcGlobal);
	void	PrintType(SIRType sType);
	void	Number(CIRFunction* pcFunction);
	void	PrintReference(CIRFunction* pcFunction, SIRReference sReference);

protected:
	void	PrintAttributes(uint32 uiAttributes);
	void	PrintBits(SIRType sType, uint64 ulliBits);
	void	PrintHex(uint64 ulliValue, int iDigits);
	void	PrintBlockName(CIRBlock* pcBlock);
	void	PrintResult(CIRFunction* pcFunction, SIRReference sResult);
	void	PrintBlock(CIRFunction* pcFunction, CIRBlock* pcBlock);
	void	PrintSource(uint32 uiSource, uint32* puiLast);
	void	PrintPhi(CIRFunction* pcFunction, SIRPhi* psPhi);
	size	PrintInstruction(CIRFunction* pcFunction, CIRBlock* pcBlock, size uiIndex);
	void	PrintCall(CIRFunction* pcFunction, CIRBlock* pcBlock, size uiFirstArg, size uiCall);
	void	PrintJump(CIRFunction* pcFunction, SIRJump* psJump);
	void	PrintTyped(CIRFunction* pcFunction, SIRType sType, SIRReference sReference);
	void	PrintString(uint8* pucBytes, size uiLength);
	void	NumberTemporary(CIRFunction* pcFunction, SIRReference sTemporary, uint32* puiNext);
};


#endif // __I_R_PRINTER_H__
