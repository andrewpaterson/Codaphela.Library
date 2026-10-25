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
#include "BaseLib/StringHelper.h"
#include "KicadSchemPinType.h"


char* gaszKicadSchemPinTypeNames[] = { "unknown", "input", "output", "bidirectional", "tri_state", "passive", "free", "unspecified", "power_in", "power_out", "open_collector", "open_emitter", "no_connect" };
char* gaszKicadSchemLabelShapeNames[] = { "none", "input", "output", "bidirectional", "tri_state", "passive", "unknown" };


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EKicadSchemPinType ParseKicadSchemPinType(char* szType)
{
	size	i;

	for (i = KSPT_Input; i <= KSPT_NoConnect; i++)
	{
		if (StringCompare(szType, gaszKicadSchemPinTypeNames[i]) == 0)
		{
			return (EKicadSchemPinType)i;
		}
	}
	return KSPT_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EKicadSchemLabelShape ParseKicadSchemLabelShape(char* szShape)
{
	size	i;

	for (i = KSLS_Input; i <= KSLS_Passive; i++)
	{
		if (StringCompare(szShape, gaszKicadSchemLabelShapeNames[i]) == 0)
		{
			return (EKicadSchemLabelShape)i;
		}
	}
	return KSLS_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* KicadSchemPinTypeName(EKicadSchemPinType eType)
{
	return gaszKicadSchemPinTypeNames[eType];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* KicadSchemLabelShapeName(EKicadSchemLabelShape eShape)
{
	return gaszKicadSchemLabelShapeNames[eShape];
}

