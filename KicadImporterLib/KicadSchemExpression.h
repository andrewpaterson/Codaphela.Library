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
#ifndef __KICAD_SCHEM_EXPRESSION_H__
#define __KICAD_SCHEM_EXPRESSION_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplatePtr.h"


enum EKicadSchemExpressionType
{
	KSET_List,
	KSET_Symbol,
	KSET_String
};


//One node of a KiCad S-expression.  A list's text is its name, the symbol after the '(', and its children are the
//elements after the name: (at 1.27 -2.54 90) is the list "at" with the three symbol children "1.27", "-2.54" and "90".
//A symbol is a bare word or number (yes, passive, 1.27) and a string is quoted ("Value").  KiCad 6 wrote some values
//as symbols that later versions quote (uuids, the generator), so the Get...Text functions accept either.
//
//Expressions are allocated and killed by CKicadSchemExpressions; Kill only frees this node's own storage.
class CKicadSchemExpression
{
protected:
	EKicadSchemExpressionType					meType;
	CChars										mszText;
	CArrayTemplatePtr<CKicadSchemExpression>	mapcChildren;
	size										muiLine;

public:
	void					Init(EKicadSchemExpressionType eType, const char* szText, size uiLength, size uiLine);
	void					Kill(void);

	void					AddChild(CKicadSchemExpression* pcChild);
	void					AppendText(char c);

	EKicadSchemExpressionType	GetType(void);
	bool					IsList(void);
	bool					IsSymbol(void);
	bool					IsString(void);
	bool					IsAtom(void);
	bool					Is(const char* szName);
	bool					IsSymbol(const char* szText);

	char*					GetText(void);
	size					GetLine(void);

	size					NumChildren(void);
	CKicadSchemExpression*	GetChild(size uiIndex);
	CKicadSchemExpression*	FindChild(const char* szName);
	CKicadSchemExpression*	FindChild(const char* szName, size uiStartIndex, size* puiIndex);
	size					CountChildren(const char* szName);
	bool					ContainsSymbol(const char* szSymbol);

	char*					GetAtomText(size uiIndex);
	bool					GetDouble(size uiIndex, float64* pfValue);
	bool					GetInt(size uiIndex, int32* piValue);

	char*					GetChildText(const char* szName);
	bool					GetChildDouble(const char* szName, float64* pfValue);
	bool					GetChildInt(const char* szName, int32* piValue);
	bool					GetChildYesNo(const char* szName, bool* pbValue);

	void					Print(CChars* psz);
};


bool ParseKicadSchemDouble(const char* szText, float64* pfValue);
bool ParseKicadSchemInt(const char* szText, int32* piValue);


#endif // __KICAD_SCHEM_EXPRESSION_H__

