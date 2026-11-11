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
#ifndef __I_R_816_CODE_H__
#define __I_R_816_CODE_H__
#include "BaseLib/ArrayChars.h"
#include "BaseLib/Chars.h"
#include "IR816BackendInternal.h"


//What a line of a W65C816 function's code is.
#define IR816CL_OTHER		0	//Blank, or anything that doesn't start with a tab.
#define IR816CL_LABEL		1	//"L12:" or a function's label, in the first column.
#define IR816CL_TABBED		2	//Starts with a tab and has a mnemonic's room: an instruction, a directive or a comment.


//A line of the code: its text and, when tabbed, its decoding (IR816Decode), the operand kept as an offset into the
//text so the line can move.
struct SIR816CodeLine
{
	CChars		szText;
	uint8		uiKind;			//IR816CL_.
	SIR816Line	sLine;
	size		uiOperand;		//sLine.szOperand's offset in szText.
};
typedef CArrayLargeTemplate<SIR816CodeLine>	CArrayIR816CodeLines;


typedef CArrayLargeTemplate<size>				CArrayIR816LineOrder;


//A W65C816 function's code as the back end wrote it (from its label on), a line at a time with each line decoded,
//for the passes over the text.  Lines are changed (Set), removed and inserted through it so their decoding is kept
//with them.  The lines stay where they are made (masLines) and the code is their order (maiOrder), so taking lines
//out and putting them in moves only the order.  As the 65C64's CIR65Code.
class CIR816Code
{
protected:
	CArrayIR816CodeLines	masLines;
	CArrayIR816LineOrder	maiOrder;

public:
	void		Init(void);
	void		Kill(void);
	void		Load(CChars* pszCode, size uiStart);
	void		Write(CChars* pszCode, size uiStart);

	size		NumLines(void);
	CChars*		GetText(size uiLine);
	const char*	Text(size uiLine);
	bool		IsLabel(size uiLine);
	bool		IsTabbed(size uiLine);
	bool		Decoded(size uiLine, SIR816Line* psLine);
	bool		Has(size uiLine, uint32 uiFlags);
	bool		Instruction(size uiLine, SIR816Line* psLine);

	void		Set(size uiLine, const char* szText);
	void		Remove(size uiLine);
	void		RemoveMarked(CArrayTemplate<uint8>* pabRemove);
	void		RemoveMarked(CArrayTemplate<bool>* pabRemove);
	void		Insert(size uiLine, const char* szText);
	void		Append(const char* szText, size uiLength);
	void		Append(const char* szText);
	void		Truncate(size uiLines);
	void		Take(CIR816Code* pcFrom);

protected:
	void			Clear(void);
	SIR816CodeLine*	Line(size uiLine);
	SIR816CodeLine*	AddLine(const char* szText, size uiLength);
	void			Decode(SIR816CodeLine* psCode);
};


#endif // __I_R_816_CODE_H__
