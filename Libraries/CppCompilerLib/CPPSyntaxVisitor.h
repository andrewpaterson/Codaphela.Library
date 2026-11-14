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
#ifndef __C_P_P_SYNTAX_VISITOR_H__
#define __C_P_P_SYNTAX_VISITOR_H__
#include "CPPSyntaxKind.h"


class CCPPSyntax;
#define CPP_SYNTAX_FORWARD(N)	class CCPPSyntax##N;
CPP_SYNTAX_KINDS(CPP_SYNTAX_FORWARD)
#undef CPP_SYNTAX_FORWARD


//Something done to a node by what kind it is, without asking its kind and casting it: pcNode->Accept(this) calls
//the Visit function of the node's own class with the node as that class.  A visitor overrides those of the kinds it
//does something for; the rest come to VisitDefault, which does nothing unless it is overridden.
class CCPPSyntaxVisitor
{
public:
	virtual void	VisitDefault(CCPPSyntax* pcNode) {}

#define CPP_SYNTAX_VISIT(N)	virtual void	Visit##N(CCPPSyntax##N* pcNode);
	CPP_SYNTAX_KINDS(CPP_SYNTAX_VISIT)
#undef CPP_SYNTAX_VISIT
};


#endif // __C_P_P_SYNTAX_VISITOR_H__
