/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

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
#ifndef __TEXT_DIFF_POLICIES_H__
#define __TEXT_DIFF_POLICIES_H__


//Blocks with more characters than this on either side aren't refined into fragments.  Myers is O(ND), and comparing
//by character takes a token per character (32 bytes on 32 bit hosts).
#define TEXT_DIFF_MAX_REFINE_LENGTH		20000


//How whitespace (spaces and tabs) is compared, as IntelliJ's "Ignore whitespaces" choice.
enum ETextDiffWhitespace
{
	TDW_Default,  //Whitespace must match.  Line breaks inside a block are compared too.
	TDW_Trim,  //Whitespace at the start and end of lines is ignored.
	TDW_Ignore  //All whitespace is ignored.
};


//How changes inside modified blocks are highlighted, as IntelliJ's "Highlight words" choice.
enum ETextDiffHighlight
{
	TDH_Lines,  //Only changed lines; blocks have no fragments.
	TDH_Words,  //Changed words, whitespace runs and punctuation.
	TDH_Chars  //Changed characters.
};


#endif // __TEXT_DIFF_POLICIES_H__
