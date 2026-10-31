/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela Assembler65Lib

Codaphela Assembler65Lib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela Assembler65Lib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela Assembler65Lib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __ASM65_SOURCE_H__
#define __ASM65_SOURCE_H__
#include "BaseLib/PrimitiveTypes.h"


//Where an assembler reads its source from, a line at a time: each pass reads it
//again from the start.  A line ends with '\n' or '\0' and stays readable until the
//next line is asked for.  The last line is the text after the last '\n' (empty if
//the source ends with one).
class CAsm65Source
{
public:
	virtual void		Rewind(void) =0;
	virtual const char*	NextLine(void) =0;	//NULL after the last line.
};


//Source text in memory.
class CAsm65TextSource : public CAsm65Source
{
protected:
	const char*		mszText;
	const char*		mszPos;
	bool			mbDone;

public:
	void			Init(const char* szText);
	void			Kill(void);

	void			Rewind(void) override;
	const char*		NextLine(void) override;
};


#endif // __ASM65_SOURCE_H__
