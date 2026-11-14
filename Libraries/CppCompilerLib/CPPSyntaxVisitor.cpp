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
#include "CPPSyntaxes.h"
#include "CPPSyntaxVisitor.h"


//Each kind's Visit comes to VisitDefault unless a visitor overrides it, and each node class gives itself to its own.
#define CPP_SYNTAX_VISIT(N)	void CCPPSyntaxVisitor::Visit##N(CCPPSyntax##N* pcNode) { VisitDefault(pcNode); }
CPP_SYNTAX_KINDS(CPP_SYNTAX_VISIT)
#undef CPP_SYNTAX_VISIT


#define CPP_SYNTAX_ACCEPT(N)	void CCPPSyntax##N::Accept(CCPPSyntaxVisitor* pcVisitor) { pcVisitor->Visit##N(this); }
CPP_SYNTAX_KINDS(CPP_SYNTAX_ACCEPT)
#undef CPP_SYNTAX_ACCEPT
