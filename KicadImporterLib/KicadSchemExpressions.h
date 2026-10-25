/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela KicadImporterLib

Codaphela KicadImporterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela KicadImporterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela KicadImporterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __KICAD_SCHEM_EXPRESSIONS_H__
#define __KICAD_SCHEM_EXPRESSIONS_H__
#include "BaseLib/ListVariable.h"
#include "KicadSchemExpression.h"


//Owns every expression parsed from one file.  Expressions are allocated individually so pointers to them stay valid.
class CKicadSchemExpressions
{
protected:
	CListVariable			macExpressions;
	CKicadSchemExpression*	mpcRoot;

public:
	void					Init(void);
	void					Kill(void);

	CKicadSchemExpression*	Add(EKicadSchemExpressionType eType, const char* szText, size uiLength, size uiLine);
	void					SetRoot(CKicadSchemExpression* pcRoot);

	CKicadSchemExpression*	GetRoot(void);
	size					NumExpressions(void);
};


#endif // __KICAD_SCHEM_EXPRESSIONS_H__

