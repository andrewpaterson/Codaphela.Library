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
	mbStaticLocal = false;
	mbLinkageExtern = false;
	mbCLinkage = false;
	mpcPackParameter = NULL;
	miQuiet = 0;
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
	psEntry->iSaved = 0;
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
// A function's body is being written: a member function that isn't static
// has this, and no parameter pack is being expanded yet.
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::BeginFunction(CCPPFunction* pcFunction)
{
	mpcFunction = pcFunction;
	mbThis = pcFunction->mpcClass && !pcFunction->mbStatic;
	mpcPackParameter = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::EndFunction(void)
{
	mpcFunction = NULL;
	mbThis = false;
}


//////////////////////////////////////////////////////////////////////////
//
// A static local's data is being written, in the function it is in: its
// initialiser has no this.
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::EnterStaticLocal(void)
{
	Enter(CPPCE_StaticLocal)->bSaved = mbThis;
	mbThis = false;
	mbStaticLocal = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPContext::LeaveStaticLocal(void)
{
	SCPPContextEntry	sEntry;

	if (Leave(CPPCE_StaticLocal, &sEntry))
	{
		mbStaticLocal = false;
		mbThis = sEntry.bSaved;
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
	miQuiet++;
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
	miQuiet--;
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
	}
	return bPassed;
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
	psEntry->bSaved = mbQuietFailed;
	miQuiet = 0;
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
		mbQuietFailed = sEntry.bSaved;
	}
}

