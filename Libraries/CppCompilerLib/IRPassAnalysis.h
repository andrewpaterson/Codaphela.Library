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
#ifndef __I_R_PASS_ANALYSIS_H__
#define __I_R_PASS_ANALYSIS_H__
#include "IRFunction.h"


class CIRModule;
class CIRPassContext;


//Something the optimiser's passes ask of the function they are run on, kept in their context (CIRPassContext): how
//big a value can be (CIRRanges), what an address is an address of (CIRAddresses).  It is not a group of passes: it
//changes nothing.  It reads the function and what else the context has of it (each temporary's definition, the
//dominators), which whatever asks must have worked out.
class CIRPassAnalysis
{
protected:
	CIRPassContext*	mpcContext;
	CIRModule*		mpcModule;
	CIRFunction*	mpcFunction;

public:
	void	Init(CIRPassContext* pcContext);
	void	Begin(CIRFunction* pcFunction);
};


#endif // __I_R_PASS_ANALYSIS_H__
