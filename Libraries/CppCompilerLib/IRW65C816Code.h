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
#include "IRW65C816BackendInternal.h"


//What a line of a W65C816 function's code is.
#define IR816CL_OTHER		0	//Blank, or anything that doesn't start with a tab.
#define IR816CL_LABEL		1	//"L12:" or a function's label, in the first column.
#define IR816CL_TABBED		2	//Starts with a tab and has a mnemonic's room: an instruction, a directive or a comment.


//A line of the code.  An instruction is its fields: sLine's mnemonic, flags and mark, and its operand, which is all of
//szText.  Any other line (a label, a directive, a comment, or a line not as the printer would write it) is its text,
//szText, and when tabbed its decoding (IR816Decode), the operand kept as an offset into the text so the line can move.
struct SIRW65C816CodeLine
{
	CChars		szText;
	uint8		uiKind;			//IR816CL_.
	uint8		uiNote;			//A pass's note on the line (IR816CN_), copied with it (AppendLine).
	bool		bText;			//The line is its text; else an instruction, szText its operand.
	SIRW65C816Line	sLine;
	size		uiOperand;		//sLine.szOperand's offset in szText.
};
typedef CArrayLargeTemplate<SIRW65C816CodeLine>	CArrayIR816CodeLines;


typedef CArrayLargeTemplate<size>				CArrayIR816LineOrder;


//A W65C816 function's code as the back end wrote it (from its label on), a line at a time with each line decoded,
//for the passes over the text.  Lines are changed (Set), removed and inserted through it so their decoding is kept
//with them.  The lines stay where they are made (masLines) and the code is their order (maiOrder), so taking lines
//out and putting them in moves only the order.  As the 65C64's CIR65Code.
class CIRW65C816Code
{
protected:
	CArrayIR816CodeLines	masLines;
	CArrayIR816LineOrder	maiOrder;
	uint8					mabKept[IR816_DIRECT_PAGE];	//The direct page bytes a debugger reads (SetKept): stores to them stay.

public:
	void		Init(void);
	void		Kill(void);
	void		Load(CChars* pszCode, size uiStart);
	void		Write(CChars* pszCode, size uiStart);
	void		PrintLine(size uiLine, CChars* psz);

	size		NumLines(void) const;
	const char*	Text(size uiLine);
	bool		IsLabel(size uiLine);
	bool		IsTabbed(size uiLine);
	bool		Decoded(size uiLine, SIRW65C816Line* psLine);
	bool		Has(size uiLine, uint32 uiFlags);
	bool		Instruction(size uiLine, SIRW65C816Line* psLine);
	bool		IsBare(size uiLine, uint16 uiMnemonic);
	bool		IsWith(size uiLine, uint16 uiMnemonic, const char* szOperand);
	bool		IsDirective(size uiLine);
	bool		IsInstructionLine(size uiLine);
	bool		IsComment(size uiLine);
	bool		IsRound(size uiLine);
	const char*	LabelName(size uiLine, size* puiLength);
	bool		IsLabelNamed(size uiLine, const char* szLabel, size uiLength);
	int64		LocalLabelNumber(size uiLine);
	const char*	Mentions(size uiLine, size* puiLength);
	int32		CountMentions(size uiFrom, const char* szText);

	void		SetInstruction(size uiLine, uint16 uiMnemonic, const char* szOperand = NULL);
	void		SetBranch(size uiLine, uint16 uiMnemonic, const char* szLabel, size uiLabel);
	void		ClearRound(size uiLine);
	void		AppendInstruction(uint16 uiMnemonic, const char* szOperand = NULL, bool bMarked = false);
	void		AppendInstruction(uint16 uiMnemonic, int64 lliOperand);
	void		AppendInstruction(SIRW65C816Line* psLine, bool bMarked);
	void		AppendLine(CIRW65C816Code* pcFrom, size uiLine);
	void		InsertInstruction(size uiLine, uint16 uiMnemonic, const char* szOperand = NULL);
	void		AppendLabel(const char* szName);
	void		AppendDirective(const char* szName, const char* szOperand = NULL);
	bool		SameLine(size uiLine, CIRW65C816Code* pcOther, size uiOther);
	bool		LinesAt(size uiLine, CIRW65C816Code* pcLines);
	void		SetNote(size uiLine, uint8 uiNote);
	uint8		Note(size uiLine);
	bool		IsNumberLine(size uiLine, uint16 uiMnemonic, const char* szOpen, const char* szClose);

	void		Set(size uiLine, const char* szText);
	void		Remove(size uiLine);
	void		RemoveMarked(CArrayTemplate<uint8>* pabRemove);
	void		RemoveMarked(CArrayTemplate<bool>* pabRemove);
	void		Insert(size uiLine, const char* szText);
	void		Append(const char* szText, size uiLength);
	void		Append(const char* szText);
	void		Truncate(size uiLines);
	void		Take(CIRW65C816Code* pcFrom);

	void		SetKept(int32 iOffset, int32 iBytes);
	bool		IsKept(int32 iOffset, int32 iBytes);
	void		MarkKept(uint8* abRead);

protected:
	void			Clear(void);
	SIRW65C816CodeLine*	Line(size uiLine);
	SIRW65C816CodeLine*	AddLine(const char* szText, size uiLength);
	SIRW65C816CodeLine*	NewLine(void);
	void			Decode(SIRW65C816CodeLine* psCode);
	void			MakeInstruction(SIRW65C816CodeLine* psCode, uint16 uiMnemonic, const char* szOperand, size uiOperand, bool bMarked);
};


#endif // __I_R_816_CODE_H__
