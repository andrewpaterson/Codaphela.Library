/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LanguageLib

Codaphela LanguageLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LanguageLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LanguageLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __META_H__
#define __META_H__
#include "Definition.h"
#include "BaseLib/ErrorTypes.h"
#include "BaseLib/EnumeratorBlock.h"


typedef CArrayTemplate<CDefinition> CArrayDefinition;


class CMeta
{
public:
	CArrayDefinition	mcDefinitions;

	void			Init(void);
	void			Kill(void);
	CDefinition*	AddDefintion(void);
	int				NumDefinitions(void);
	CDefinition*	GetDefinition(int i);
	CDefinition*	GetEntryDefinition(void);

	void			Dump(void);
};

extern CMeta*	gpcCurrentMeta;

#endif //__META_H__