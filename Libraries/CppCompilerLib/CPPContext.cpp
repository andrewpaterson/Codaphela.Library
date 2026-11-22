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
#include "CPPContext.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::Init(void)
{
	mpcBindings = NULL;
	mpcDeclaringNamespace = NULL;
	mpcFunction = NULL;
	mbThis = false;
	mpcObject = NULL;
	mbLinkageExtern = false;
	mbCLinkage = false;
	miUnevaluated = 0;
	miQuiet = 0;
	miKeeping = 0;
	miKeptQuiet = 0;
	mbQuietFailed = false;
	masEntered.Init();
	mbBroken = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::Kill(void)
{
	masEntered.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPPContextEntry* CCPPContext::Enter(ECPPContextEntry eKind)
{
	SCPPContextEntry*	psEntry;

	psEntry = masEntered.Add();
	psEntry->eKind = eKind;
	psEntry->pvSaved = NULL;
	psEntry->pvOtherSaved = NULL;
	psEntry->iSaved = 0;
	psEntry->iOtherSaved = 0;
	psEntry->bSaved = false;
	psEntry->bOtherSaved = false;
	return psEntry;
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the last thing entered off into psDest.  False, and the context
// is broken, if it isn't an eKind: what was entered is left where it is.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPContext::Leave(ECPPContextEntry eKind, SCPPContextEntry* psDest)
{
	SCPPContextEntry*	psEntry;

	psEntry = masEntered.Tail();
	if (!psEntry || (psEntry->eKind != eKind))
	{
		mbBroken = true;
		return false;
	}
	*psDest = *psEntry;
	masEntered.RemoveTail();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The template arguments names mean are pcBindings (NULL for none) until
// LeaveBindings.
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::EnterBindings(CCPPTemplateArguments* pcBindings)
{
	Enter(CPPCE_Bindings)->pvSaved = mpcBindings;
	mpcBindings = pcBindings;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::LeaveBindings(void)
{
	SCPPContextEntry	sEntry;

	if (Leave(CPPCE_Bindings, &sEntry))
	{
		mpcBindings = (CCPPTemplateArguments*)sEntry.pvSaved;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Declarations are in pcNamespace (NULL for the global one) until
// LeaveNamespace.
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::EnterNamespace(CCPPScope* pcNamespace)
{
	Enter(CPPCE_Namespace)->pvSaved = mpcDeclaringNamespace;
	mpcDeclaringNamespace = pcNamespace;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::LeaveNamespace(void)
{
	SCPPContextEntry	sEntry;

	if (Leave(CPPCE_Namespace, &sEntry))
	{
		mpcDeclaringNamespace = (CCPPScope*)sEntry.pvSaved;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A linkage specification's declarations: extern "C" or not, and extern
// themselves if it has no braces.  [dcl.link]
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::EnterLinkage(bool bCLinkage, bool bExtern)
{
	SCPPContextEntry*	psEntry;

	psEntry = Enter(CPPCE_Linkage);
	psEntry->bSaved = mbCLinkage;
	psEntry->bOtherSaved = mbLinkageExtern;
	mbCLinkage = bCLinkage;
	mbLinkageExtern = bExtern;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::LeaveLinkage(void)
{
	SCPPContextEntry	sEntry;

	if (Leave(CPPCE_Linkage, &sEntry))
	{
		mbCLinkage = sEntry.bSaved;
		mbLinkageExtern = sEntry.bOtherSaved;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A function is being written, or its parameters and locals made to type
// what is in it, until LeaveFunction: a member function that isn't static
// has this.  The function it was entered from, and whether that had a
// this, come back when it is left.
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::EnterFunction(CCPPFunction* pcFunction)
{
	SCPPContextEntry*	psEntry;

	psEntry = Enter(CPPCE_Function);
	psEntry->pvSaved = mpcFunction;
	psEntry->bSaved = mbThis;
	mpcFunction = pcFunction;
	mbThis = pcFunction->mpcClass && !pcFunction->mbStatic;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::LeaveFunction(void)
{
	SCPPContextEntry	sEntry;

	if (Leave(CPPCE_Function, &sEntry))
	{
		mpcFunction = (CCPPFunction*)sEntry.pvSaved;
		mbThis = sEntry.bSaved;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An aggregate of pcClass is being filled where it is made: the default
// member initialisers of the members its list leaves out are written in the
// class, and their this is the aggregate (struct S { int a = 1; int b = a +
// 1; }; S s = { 10 };).  [dcl.init.aggr] 7
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::EnterObject(CCPPClass* pcClass)
{
	SCPPContextEntry*	psEntry;

	psEntry = Enter(CPPCE_Object);
	psEntry->pvSaved = mpcObject;
	psEntry->bSaved = mbThis;
	mpcObject = pcClass;
	mbThis = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::LeaveObject(void)
{
	SCPPContextEntry	sEntry;

	if (Leave(CPPCE_Object, &sEntry))
	{
		mpcObject = (CCPPClass*)sEntry.pvSaved;
		mbThis = sEntry.bSaved;
	}
}


CCPPClass* CCPPContext::GetObject(void) const { return mpcObject; }


//////////////////////////////////////////////////////////////////////////
//
// What is written until LeaveNoThis has no this, in whatever function it is
// written: the data of a static local, or of a global defined while a
// member function is written (a template's static member, first used
// there).
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::EnterNoThis(void)
{
	Enter(CPPCE_NoThis)->bSaved = mbThis;
	mbThis = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::LeaveNoThis(void)
{
	SCPPContextEntry	sEntry;

	if (Leave(CPPCE_NoThis, &sEntry))
	{
		mbThis = sEntry.bSaved;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An operand that isn't evaluated (sizeof's, decltype's, noexcept's) is
// being typed until LeaveUnevaluated: a member is named with no object, and
// nothing it names is used.  One inside another is the same.  [expr] 8
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::EnterUnevaluated(void)
{
	Enter(CPPCE_Unevaluated);
	miUnevaluated++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::LeaveUnevaluated(void)
{
	SCPPContextEntry	sEntry;

	if (Leave(CPPCE_Unevaluated, &sEntry))
	{
		miUnevaluated--;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Errors are not reported until EndQuiet.  True if nothing was quiet before:
// whether it fails is then counted from here.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPContext::BeginQuiet(void)
{
	bool	bOutermost;

	bOutermost = miQuiet == 0;
	if (bOutermost)
	{
		mbQuietFailed = false;
	}
	Enter(CPPCE_Quiet);
	miQuiet++;
	miKeptQuiet++;
	return bOutermost;
}


//////////////////////////////////////////////////////////////////////////
//
// True if no error was made since the outermost BeginQuiet (or since the trial
// it is in began).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPContext::EndQuiet(void)
{
	SCPPContextEntry	sEntry;

	if (Leave(CPPCE_Quiet, &sEntry))
	{
		miQuiet--;
		miKeptQuiet--;
	}
	return !mbQuietFailed;
}


//////////////////////////////////////////////////////////////////////////
//
// Quiet code whose failing is its own.  True if nothing was quiet before.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPContext::BeginTrial(void)
{
	bool	bOutermost;

	bOutermost = miQuiet == 0;
	Enter(CPPCE_Trial)->bSaved = mbQuietFailed;
	mbQuietFailed = false;
	miQuiet++;
	miKeptQuiet++;
	return bOutermost;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the trial made no error.  What was quiet around it has failed or not
// as it had before the trial.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPContext::EndTrial(void)
{
	SCPPContextEntry	sEntry;
	bool				bPassed;

	bPassed = !mbQuietFailed;
	if (Leave(CPPCE_Trial, &sEntry))
	{
		mbQuietFailed = sEntry.bSaved;
		miQuiet--;
		miKeptQuiet--;
	}
	return bPassed;
}


//////////////////////////////////////////////////////////////////////////
//
// What is worked out until LeaveKeeping has its errors kept with it: an
// error made there is recorded (IsKeeping), in quiet code too, though it is
// noted as quiet code's failing as it would be.  Not one made in quiet code
// that begins inside the part: that is judged by what it returns, as ever.
// Being quiet, and what has failed, are as they were.
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::EnterKeeping(void)
{
	Enter(CPPCE_Keeping)->iSaved = miKeptQuiet;
	miKeptQuiet = 0;
	miKeeping++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::LeaveKeeping(void)
{
	SCPPContextEntry	sEntry;

	if (Leave(CPPCE_Keeping, &sEntry))
	{
		miKeptQuiet = sEntry.iSaved;
		miKeeping--;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Errors are reported until EndLoud, whatever is quiet around it.
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::BeginLoud(void)
{
	SCPPContextEntry*	psEntry;

	psEntry = Enter(CPPCE_Loud);
	psEntry->iSaved = miQuiet;
	psEntry->iOtherSaved = miKeptQuiet;
	psEntry->bSaved = mbQuietFailed;
	miQuiet = 0;
	miKeptQuiet = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::EndLoud(void)
{
	SCPPContextEntry	sEntry;

	if (Leave(CPPCE_Loud, &sEntry))
	{
		miQuiet = sEntry.iSaved;
		miKeptQuiet = sEntry.iOtherSaved;
		mbQuietFailed = sEntry.bSaved;
	}
}


CCPPTemplateArguments* CCPPContext::GetBindings(void) const { return mpcBindings; }
CCPPScope* CCPPContext::GetNamespace(void) const { return mpcDeclaringNamespace; }
bool CCPPContext::IsCLinkage(void) const { return mbCLinkage; }
//////////////////////////////////////////////////////////////////////////
//
// eWhat is being worked out of pvOn (an expression), with the template
// arguments there are, until LeaveWorking.  False, and nothing is entered,
// if it is being worked out already: it has been asked for from inside
// itself.  The same expression with other template arguments is another
// thing (one instance's constant may need another's, written by the same
// tokens).  pvOn is NULL where there is one thing of the kind at a time,
// whatever the template arguments.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPContext::EnterWorking(ECPPWorking eWhat, void* pvOn)
{
	SCPPContextEntry*	psEntry;

	if (IsWorking(eWhat, pvOn))
	{
		return false;
	}
	psEntry = Enter(CPPCE_Working);
	psEntry->iSaved = eWhat;
	psEntry->pvSaved = pvOn;
	psEntry->pvOtherSaved = mpcBindings;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::LeaveWorking(void)
{
	SCPPContextEntry	sEntry;

	Leave(CPPCE_Working, &sEntry);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPContext::IsWorking(ECPPWorking eWhat, void* pvOn) const
{
	SCPPContextEntry*	psEntry;
	size				i;

	for (i = masEntered.NumElements(); i > 0; i--)
	{
		psEntry = ((CArrayCPPContextEntries*)&masEntered)->Get(i - 1);
		if ((psEntry->eKind == CPPCE_Working) && (psEntry->iSaved == eWhat) && (psEntry->pvSaved == pvOn) && (!pvOn || (psEntry->pvOtherSaved == mpcBindings)))
		{
			return true;
		}
	}
	return false;
}


bool CCPPContext::IsLinkageExtern(void) const { return mbLinkageExtern; }
CCPPFunction* CCPPContext::GetFunction(void) const { return mpcFunction; }
bool CCPPContext::HasThis(void) const { return mbThis; }
bool CCPPContext::IsQuiet(void) const { return miQuiet > 0; }
bool CCPPContext::IsKeeping(void) const { return (miKeeping > 0) && (miKeptQuiet == 0); }
bool CCPPContext::IsUnevaluated(void) const { return miUnevaluated > 0; }
bool CCPPContext::IsBroken(void) const { return mbBroken || (masEntered.NumElements() != 0); }
