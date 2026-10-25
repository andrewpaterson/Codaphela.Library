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
#include "KicadSchemExpressions.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemExpressions::Init(void)
{
	macExpressions.Init();
	mpcRoot = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemExpressions::Kill(void)
{
	size					i;
	size					uiNumElements;
	CKicadSchemExpression*	pcExpression;

	uiNumElements = macExpressions.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcExpression = (CKicadSchemExpression*)macExpressions.Get(i);
		pcExpression->Kill();
	}
	macExpressions.Kill();
	mpcRoot = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CKicadSchemExpression* CKicadSchemExpressions::Add(EKicadSchemExpressionType eType, const char* szText, size uiLength, size uiLine)
{
	CKicadSchemExpression*	pcExpression;

	pcExpression = (CKicadSchemExpression*)macExpressions.Add(sizeof(CKicadSchemExpression));
	pcExpression->Init(eType, szText, uiLength, uiLine);
	return pcExpression;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemExpressions::SetRoot(CKicadSchemExpression* pcRoot)
{
	mpcRoot = pcRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CKicadSchemExpression* CKicadSchemExpressions::GetRoot(void)
{
	return mpcRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CKicadSchemExpressions::NumExpressions(void)
{
	return macExpressions.NumElements();
}

