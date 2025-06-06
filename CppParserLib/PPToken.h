/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#ifndef __P_P_TOKEN_H__
#define __P_P_TOKEN_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Define.h"
#include "BaseLib/MemoryStackExtended.h"
#include "BaseLib/Constructable.h"
#include "GeneralToken.h"


class CPPTokenMemory;
class CPPToken
{
protected:
	int32		miLine;
	int16		miColumn;
	int16		miUsage;

#ifdef DEBUG
	int64		miTokenNum;
	char*		mszFileName;
#endif // DEBUG

public:
	virtual void			Init(int iLine, int iColumn, char* szFileName);
	virtual void			Kill(void) =0;

	virtual const char*		ClassName(void) =0;
	virtual unsigned int	ClassSize(void) =0;
			char*			ShortFileName(void);

	virtual bool			IsDirective(void);
	virtual bool			IsText(void);
	virtual bool			IsWhitespace(void);
	virtual bool			IsReplacement(void);
	virtual bool			IsHash(void);
	virtual bool			IsTokenListHolder(void);
	virtual bool			IsLine(void);
	virtual bool			IsTokenReplacementsHolder(void);
	virtual bool			IsFile(void);
	virtual bool			IsBlock(void);
	virtual bool			IsUnknown(void);
	virtual bool			IsEmpty(void) =0;
			void			Set(int iLine, int iColumn);
	virtual char*			Print(CChars* psz) =0;
	virtual void			Copy(CPPToken* pcSource, CPPTokenMemory* pcTokens) =0;
	virtual bool			Equals(CPPToken* pcOther) =0;
	virtual void			Use(void);
			void			Unuse(void);

			int				Line(void);
			int				Column(void);
	virtual bool			NeedsNewLine(void);
			void			Dump(void);
};


typedef CArrayTemplate<CPPToken*> CArrayPPTokenPtrs;


void PPSetBreakOnTokenInit(int64 iTokenNum);


#endif // !__P_P_TOKEN_H__

