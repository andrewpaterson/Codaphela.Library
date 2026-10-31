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
#ifndef __CPP65_ASSEMBLY_H__
#define __CPP65_ASSEMBLY_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"
#include "Assembler65Lib/Asm65Source.h"
#include "SpillStore.h"


#define CPP65_ASSEMBLY_MARKER	'\x01'		//Around a function's index in the text (IR65_FRONT_END_MARKER).


//A unit's assembly as the generator leaves it: the text (code, thunks, data and
//strings) with a marker (CPP65_ASSEMBLY_MARKER index CPP65_ASSEMBLY_MARKER) where
//each function's code goes, and the functions' code in a spill store, each in pieces
//(PutCode) of at most the store's MaxDataSize.  It is read a line at a time by the
//assembler (CAsm65Source) or written whole (Text, Write), and is never all in memory.
//Kill takes the functions' code out of the store.
class CCPP65Assembly : public CAsm65Source
{
protected:
	CLargeChars						mszText;
	CSpillStore*					mpcStore;
	CArrayLargeTemplate<uint16>		mauiPieces;		//Per function: how many pieces its code is in.

	lsize							muiPos;			//Reading: where in mszText,
	lsize							muiFunction;	//the function being read (MAX_LSIZE for none),
	uint16							muiPiece;		//its next piece,
	CChars							mszPiece;		//the piece being read,
	size							muiPiecePos;	//where in it,
	CChars							mszLine;		//and the line read.
	bool							mbDone;

public:
	void			Init(CSpillStore* pcStore);
	void			Kill(void);

	CLargeChars*	GetText(void);
	void			SetPieces(CArrayLargeTemplate<uint16>* pauiPieces);
	bool			IsEmpty(void);

	void			Rewind(void) override;
	const char*		NextLine(void) override;

	void			Text(CChars* pszDest);
	bool			Write(const char* szFilename);

	static bool		PutCode(CSpillStore* pcStore, lsize uiFunction, const char* szCode, lsize uiLength, uint16* puiPieces);
	static bool		GetCode(CSpillStore* pcStore, lsize uiFunction, uint16 uiPieces, CChars* pszDest);
	static void		RemoveCode(CSpillStore* pcStore, lsize uiFunction, uint16 uiFrom, uint16 uiTo);

protected:
	static size		Key(uint8* puiKey, lsize uiFunction, uint16 uiPiece);
	bool			LoadPiece(void);
};


#endif // __CPP65_ASSEMBLY_H__
