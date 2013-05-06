/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __ASCII_NODE_H__
#define __ASCII_NODE_H__
#include "Chars.h"

//The first 33 ASCII characters are not representable so to save space they are ignored.  127-33 = 94
#define ASCII_NODE_MAX_CHARS	95
#define ASCII_NODE_START_CHAR	32


//This node does not represent a letter.  The arrays mapcChildren and maiWordEnds index letters.

//For example (and not subtracting 33)
//Node.mapcChildren['A'] = not null is the start of a word beginning 'A' but being more than just "A"
//Node.maiWordEnds['A'] = positive number is the complete word "A".  The number indexes mcWords.

//Node.mapcChildren['A'].mapcChildren['n'].maiWordEnds['d'] = positive number is the complete word "And".
//Node.mapcChildren['A'].mapcChildren['n'].mapcChildren['d'] == NULL if there are no longer words beginning "And".


class CASCIINode
{
public:
	CASCIINode*		mpcParent;
	int				miParentLetter;
	CASCIINode*		mapcChildren[ASCII_NODE_MAX_CHARS];  //The characters !(33) to ~(126).  ie: add 33 to the index and get the character.
	int				maiWordEnds[ASCII_NODE_MAX_CHARS];

	void			Init(CASCIINode* pcParent, int iParentLetter);

	BOOL			IsEmpty(void);
};


#endif // __ASCII_NODE_H__

