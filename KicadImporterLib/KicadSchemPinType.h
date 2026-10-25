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
#ifndef __KICAD_SCHEM_PIN_TYPE_H__
#define __KICAD_SCHEM_PIN_TYPE_H__


//A library pin's electrical type, the symbol after (pin: (pin input line (at ...) ...).
enum EKicadSchemPinType
{
	KSPT_Unknown,
	KSPT_Input,
	KSPT_Output,
	KSPT_Bidirectional,
	KSPT_TriState,
	KSPT_Passive,
	KSPT_Free,
	KSPT_Unspecified,
	KSPT_PowerIn,
	KSPT_PowerOut,
	KSPT_OpenCollector,
	KSPT_OpenEmitter,
	KSPT_NoConnect
};


//The shape of a global or hierarchical label and of a sheet pin: (shape input) or (pin "D0" input ...).
enum EKicadSchemLabelShape
{
	KSLS_None,
	KSLS_Input,
	KSLS_Output,
	KSLS_Bidirectional,
	KSLS_TriState,
	KSLS_Passive,
	KSLS_Unknown
};


EKicadSchemPinType		ParseKicadSchemPinType(char* szType);
EKicadSchemLabelShape	ParseKicadSchemLabelShape(char* szShape);
char*					KicadSchemPinTypeName(EKicadSchemPinType eType);
char*					KicadSchemLabelShapeName(EKicadSchemLabelShape eShape);


#endif // __KICAD_SCHEM_PIN_TYPE_H__

