/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __UNMANAGED_CLASSES_H__
#define __UNMANAGED_CLASSES_H__
#include "Class.h"


class CClasses;
class CUnmanagedClasses
{
private:
	CClasses*	mpcClasses;
	size		muiNumUnmanagedClasses;

protected:
	CClass*	mpcUndefined;
	CClass*	mpcVoidPointer;

	CClass*	mpcInt8;
	CClass*	mpcInt16;
	CClass*	mpcInt32;
	CClass*	mpcInt64;
	CClass*	mpcUInt8;
	CClass*	mpcUInt16;
	CClass*	mpcUInt32;
	CClass*	mpcUInt64;
	CClass*	mpcFloat32;
	CClass*	mpcFloat64;
	CClass*	mpcM2float32;
	CClass*	mpcM3float32;
	CClass*	mpcM4float32;
	CClass*	mpcM2float64;
	CClass*	mpcM3float64;
	CClass*	mpcM4float64;
	CClass*	mpcM2int32;
	CClass*	mpcM3int32;
	CClass*	mpcM4int32;
	CClass*	mpcM4x4float32;
	CClass*	mpcBool;
	CClass*	mpcVoid;
	CClass*	mpcChar8;
	CClass*	mpcChar16;

	CClass*	mpcString;
	CClass*	mpcStringImmutable;
	CClass*	mpcNumber;
	CClass*	mpcDate;
	CClass*	mpcDateTime;
	CClass*	mpcEnum;

	CClass*	mpcBit;
	CClass*	mpcCrumb;
	CClass*	mpcTribble;
	CClass*	mpcNybble;
	CClass*	mpcNickle;
	CClass*	mpcSixbits;

public:
	void		Init(CClasses* pcClasses);
	void		Kill(void);

	CClass*		GetUndefined(void) const;
	CClass*		GetVoidPointer(void) const;
	CClass*		GetByte(void) const;
	CClass*		GetShort(void) const;
	CClass*		GetInt(void) const;
	CClass*		GetLong(void) const;
	CClass*		GetUByte(void) const;
	CClass*		GetUShort(void) const;
	CClass*		GetUInt(void) const;
	CClass*		GetULong(void) const;
	CClass*		GetFloat(void) const;
	CClass*		GetDouble(void) const;
	CClass*		GetFloat2(void) const;
	CClass*		GetFloat3(void) const;
	CClass*		GetFloat4(void) const;
	CClass*		GetDouble2(void) const;
	CClass*		GetDouble3(void) const;
	CClass*		GetDouble4(void) const;
	CClass*		GetInt2(void) const;
	CClass*		GetInt3(void) const;
	CClass*		GetInt4(void) const;
	CClass*		GetFloat4x4(void) const;
	CClass*		GetBool(void) const;
	CClass*		GetVoid(void) const;
	CClass*		GetChar(void) const;
	CClass*		GetWidechar(void) const;
	CClass*		GetString(void) const;
	CClass*		GetStringImmutable(void) const;
	CClass*		GetNumber(void) const;
	CClass*		GetDate(void) const;
	CClass*		GetDateTime(void) const;
	CClass*		GetBit(void) const;
	CClass*		GetCrumb(void) const;
	CClass*		GetTribble(void) const;
	CClass*		GetNybble(void) const;
	CClass*		GetNickle(void) const;
	CClass*		GetSixbits(void) const;
	CClass*		GetEnum(void) const;

protected:
	CClass*		AddClassByPrettyName(EPrimitiveType eType);
	CClass*		AddClassByCPPName(EPrimitiveType eType);
	CClass*		AddClass(EPrimitiveType eType, const char* szName, size iSize);
};


#endif // __UNMANAGED_CLASSES_H__

