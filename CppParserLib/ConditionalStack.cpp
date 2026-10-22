/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "ConditionalStack.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CConditionalStack::Init(void)
{
	mcStack.Init();
	mbParsing = true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CConditionalStack::Kill(void)
{
	mcStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CConditionalStack::IsParsing(void)
{
	return mbParsing;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CConditionalStack::IsParentParsing(void)
{
	size					i;
	size					iNumElements;

	//True if every group enclosing the innermost one is being parsed.
	iNumElements = mcStack.NumElements();
	for (i = 0; i + 1 < iNumElements; i++)
	{
		if (!mcStack.Get(i)->bEvaluated)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CConditionalStack::IsElseIfEvaluated(void)
{
	size					i;
	SDefineConditional*		psConditional;
	size					iNumElements;

	//An #elif's expression is only evaluated if every enclosing group is being parsed and no earlier branch was taken.
	iNumElements = mcStack.NumElements();
	if (iNumElements == 0)
	{
		return true;
	}

	for (i = 0; i < iNumElements - 1; i++)
	{
		psConditional = mcStack.Get(i);
		if (!psConditional->bEvaluated)
		{
			return false;
		}
	}

	psConditional = mcStack.Tail();
	return !psConditional->bElseIfPassed;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CConditionalStack::CalculateIsParsing(void)
{
	size					i;
	SDefineConditional*		psConditional;

	mbParsing = true;
	for (i = 0; i < mcStack.NumElements(); i++)
	{
		psConditional = mcStack.Get(i);
		mbParsing &= psConditional->bEvaluated;  //If any evaluate to false, we are not parsing.
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CConditionalStack::PushIfNotDefined(bool bEvaluated)
{
	Push(CT_IfNotDefined, bEvaluated, bEvaluated);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CConditionalStack::PushIfDefined(bool bEvaluated)
{
	Push(CT_IfDefined, bEvaluated, bEvaluated);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CConditionalStack::SwapForElse(void)
{
	SDefineConditional*		psConditional;

	psConditional = mcStack.Tail();
	if (psConditional)
	{
		if (psConditional->bElseIfPassed)
		{
			psConditional->bEvaluated = false;
		}
		else
		{
			psConditional->bEvaluated = !psConditional->bEvaluated;
			psConditional->eType = CT_Else;

			if (psConditional->bEvaluated)
				psConditional->bElseIfPassed = true;
		}
	}

	CalculateIsParsing();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CConditionalStack::SwapForElseIf(bool bEvaluated)
{
	SDefineConditional*		psConditional;

	psConditional = mcStack.Tail();
	if (psConditional)
	{
		if (psConditional->bElseIfPassed)
		{
			psConditional->bEvaluated = false;
		}
		else
		{
			psConditional->bEvaluated = bEvaluated;  //Uh... haven't thought about this, could be wrong.
			psConditional->eType = CT_ElseIf;

			if (psConditional->bEvaluated)
				psConditional->bElseIfPassed = true;
		}
	}

	CalculateIsParsing();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CConditionalStack::PushIf(bool bEvaluated)
{
	Push(CT_If, bEvaluated, bEvaluated);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CConditionalStack::PopEndIf(void)
{
	if (mcStack.IsNotEmpty())
	{
		mcStack.Pop();
	}

	CalculateIsParsing();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CConditionalStack::Push(EConditionalType eType, bool bEvaluated, bool bElseIfPassed)
{
	SDefineConditional*		psConditional;


	psConditional = mcStack.Push();
	psConditional->bEvaluated = bEvaluated;
	psConditional->eType = eType;
	psConditional->bElseIfPassed = bElseIfPassed;

	CalculateIsParsing();
}

