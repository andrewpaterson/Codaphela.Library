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
#ifndef __C_P_P_DIAGNOSTICS_H__
#define __C_P_P_DIAGNOSTICS_H__
#include "BaseLib/Chars.h"
#include "CPPErrors.h"


class CCPPTokeniser;
class CCPPContext;
class CCPPSyntax;
class CCPPCompileStatistics;
struct SPreprocessedToken;


//What a unit's compiling says: its errors, its warnings, and when it says nothing.  An error is at a node or a token
//of the unit, as "File (line, column): message [token]" with where the macro that made the token was used.  In quiet
//code (CCPPContext) an error is only noted: what is done there is judged by what it returns.  A trial is quiet code
//whose failing is its own, and a loud part is said whatever is quiet around it.  A warning is said once for a token
//and doesn't fail the unit.
class CCPPDiagnostics
{
protected:
	CCPPTokeniser*			mpcTokeniser;
	CCPPContext*			mpcContext;		//Where the compiler is: whether it is quiet.
	CCPPCompileStatistics*	mpcStatistics;	//Where quiet trials are counted (SetStatistics).
	CCPPErrors				mcErrors;
	CCPPErrors				mcWarnings;		//Each at most once (by token); they don't fail the unit.

public:
	void		Init(CCPPTokeniser* pcTokeniser, CCPPContext* pcContext, CCPPCompileStatistics* pcStatistics);
	void		Kill(void);
	void		SetStatistics(CCPPCompileStatistics* pcStatistics);

	bool		Error(CCPPSyntax* pcNode, const char* szMessage);
	bool		ErrorAt(int iToken, const char* szMessage);
	bool		ReportError(CCPPSyntax* pcNode, const char* szMessage);
	bool		ReportErrorAt(int iToken, const char* szMessage);
	bool		ReportOnce(CCPPSyntax* pcNode, const char* szMessage);
	void		Warning(CCPPSyntax* pcNode, const char* szMessage);
	bool		HasWarning(CCPPSyntax* pcNode, const char* szText);

	void		BeginQuiet(void);
	bool		EndQuiet(void);
	void		BeginTrial(void);
	bool		EndTrial(void);
	void		BeginLoud(void);
	void		EndLoud(void);

	CCPPErrors*	GetErrors(void);
	CCPPErrors*	GetWarnings(void);
	size		NumErrors(void) const;

protected:
	void		AppendMacroUse(SPreprocessedToken* psSource, CChars* pszMessage);
};


#endif // __C_P_P_DIAGNOSTICS_H__

