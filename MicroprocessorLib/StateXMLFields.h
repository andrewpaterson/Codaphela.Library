/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela MicroprocessorLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MicroprocessorLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MicroprocessorLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __STATE_XML_FIELDS_H__
#define __STATE_XML_FIELDS_H__
#include <stddef.h>
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/XMLWriter.h"
#include "BaseLib/MarkupTag.h"


//Reads one attribute of a chip's saved state.  Each logs an error naming the tag and attribute and returns false (or
//-1) if the attribute is missing or out of range.
bool	LoadStateBool(CMarkupTag* pcTag, const char* szName, bool* pbValue);
bool	LoadStateInt64(CMarkupTag* pcTag, const char* szName, int64 iMinimum, int64 iMaximum, int64* piValue);
int		LoadStateName(CMarkupTag* pcTag, const char* szName, const char** aszNames, int iNumNames);


enum EStateXMLFieldType
{
	SXFT_Bool,
	SXFT_Unsigned,
	SXFT_Signed,
};


//One field of a plain state struct saved as an attribute: the one definition both saving and loading use.  The range
//loaded is the field's type's; an 8 byte unsigned field is saved as its signed bits.
struct SStateXMLField
{
	const char*			szName;
	size				uiOffset;
	uint8				uiBytes;
	EStateXMLFieldType	eType;
};


#define STATE_XML_FIELD(Struct, Member, Name, Type)		{ Name, offsetof(Struct, Member), (uint8)sizeof(((Struct*)NULL)->Member), Type }


void	SaveStateXMLFields(CXMLWriter* pcWriter, const SStateXMLField* asFields, size uiNumFields, void* pvState);
bool	LoadStateXMLFields(CMarkupTag* pcTag, const SStateXMLField* asFields, size uiNumFields, void* pvState);


#endif // __STATE_XML_FIELDS_H__

