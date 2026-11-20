/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __C_P_P_CONTEXT_H__
#define __C_P_P_CONTEXT_H__
#include "BaseLib/ArrayTemplate.h"
#include "CPPScope.h"
#include "CPPSymbol.h"
#include "CPPFunction.h"
#include "CPPTemplate.h"


enum ECPPContextEntry
{
	CPPCE_Bindings,
	CPPCE_Namespace,
	CPPCE_Linkage,
	CPPCE_StaticLocal,
	CPPCE_Object,
	CPPCE_Trial,
	CPPCE_Loud,
};


//What entering something put aside, to be put back when it is left.
struct SCPPContextEntry
{
	ECPPContextEntry	eKind;
	void*				pvSaved;
	int					iSaved;
	bool				bSaved;
	bool				bOtherSaved;
};


typedef CArrayTemplate<SCPPContextEntry>	CArrayCPPContextEntries;


//Where the compiler is in the unit: what a declaration or an expression means depends on it.  The template
//arguments of what is being generated or laid out, the namespace declarations are in, the linkage, and the
//function being written.  A part of it is entered and left (EnterBindings and LeaveBindings), and what was
//there before comes back when it is left: nothing is saved and put back by hand.  Leaving what wasn't the
//last thing entered is a mistake in the compiler, and is kept (IsBroken) for the unit to report.
//
//It is also where errors go.  In quiet code they are not reported: what is done there is judged by what it
//returns, and a declaration the compiler can't do yet is only an error when it is used.  Quiet code in quiet
//code is the same quiet code.  A trial is quiet code whose failing is its own (EndTrial says if it did), and a
//loud part is reported whatever is around it.
class CCPPContext
{
protected:
	CCPPTemplateArguments*	mpcBindings;			//The template arguments of what is being generated or laid out.
	CCPPScope*				mpcDeclaringNamespace;	//The namespace declarations are in, NULL for the global one: of what has no symbol (an operator).
	CCPPFunction*			mpcFunction;			//The function being written, or NULL.
	bool					mbThis;					//It is a member function with this (not a global's or static local's initialiser).
	CCPPClass*				mpcObject;				//An aggregate being filled, which is this for its default member initialisers; else NULL.
	bool					mbStaticLocal;			//A static local's data is being written.
	bool					mbLinkageExtern;		//In extern "C" without braces: declarations are extern.
	bool					mbCLinkage;				//In extern "C": functions declared have C linkage.
	int						miQuiet;				//Above 0, errors are not reported, only noted in mbQuietFailed.
	bool					mbQuietFailed;
	CArrayCPPContextEntries	masEntered;
	bool					mbBroken;

public:
	void					Init(void);
	void					Kill(void);

	void					EnterBindings(CCPPTemplateArguments* pcBindings);
	void					LeaveBindings(void);
	CCPPTemplateArguments*	GetBindings(void) const;

	void					EnterNamespace(CCPPScope* pcNamespace);
	void					LeaveNamespace(void);
	CCPPScope*				GetNamespace(void) const;

	void					EnterLinkage(bool bCLinkage, bool bExtern);
	void					LeaveLinkage(void);
	bool					IsCLinkage(void) const;
	bool					IsLinkageExtern(void) const;

	void					BeginFunction(CCPPFunction* pcFunction);
	void					EndFunction(void);
	void					NoThis(void) { mbThis = false; }
	CCPPFunction*			GetFunction(void) const;
	bool					HasThis(void) const;

	void					EnterObject(CCPPClass* pcClass);
	void					LeaveObject(void);
	CCPPClass*				GetObject(void) const;
	void					EnterStaticLocal(void);
	void					LeaveStaticLocal(void);
	bool					IsStaticLocal(void) const;


	bool					BeginQuiet(void);
	bool					EndQuiet(void);
	bool					BeginTrial(void);
	bool					EndTrial(void);
	void					BeginLoud(void);
	void					EndLoud(void);
	bool					IsQuiet(void) const;
	void					SetFailed(void) { mbQuietFailed = true; }

	bool					IsBroken(void) const;

protected:
	SCPPContextEntry*		Enter(ECPPContextEntry eKind);
	bool					Leave(ECPPContextEntry eKind, SCPPContextEntry* psDest);
};


#endif // __C_P_P_CONTEXT_H__
