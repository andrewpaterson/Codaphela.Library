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
#ifndef __C_P_P_TARGET_HOST_H__
#define __C_P_P_TARGET_HOST_H__
#include "CPPTarget.h"


//The host: code that runs on the PC, only checked (CCPPSemanticPass) and never generated.  Its sizes are MSVC's
//x64 ones (long is 4 bytes, as it is on Windows), and its project defines its own macros.
class CCPPTargetHost : public CCPPTarget
{
public:
	ECPPTarget	GetTarget(void) override;
	const char*	GetConfigurationName(void) override;
	int			GetIntBytes(void) override;
	int			GetPointerBytes(void) override;
};


#endif // __C_P_P_TARGET_HOST_H__

