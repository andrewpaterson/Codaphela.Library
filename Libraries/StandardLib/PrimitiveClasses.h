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
#ifndef __PRIMITIVE_CLASSES_H__
#define __PRIMITIVE_CLASSES_H__
#include "BaseLib/GlobalMemory.h"
#include "Class.h"


class CClasses;
class CPrimitiveClasses
{
private:
	CClasses* mpcClasses;

protected:
	CClass* mpcInt8;
	CClass* mpcInt16;
	CClass* mpcInt32;
	CClass* mpcInt64;
	CClass* mpcUInt8;
	CClass* mpcUInt16;
	CClass* mpcUInt32;
	CClass* mpcUInt64;
	CClass* mpcFloat32;
	CClass* mpcFloat64;
	CClass* mpcBool;
	CClass* mpcVoid;
	CClass* mpcChar8;
	CClass* mpcChar16;

public:
	void		Init(CClasses* pcClasses);
	void		Kill(void);

	CClass*		GetByte(void);
	CClass*		GetShort(void);
	CClass*		GetInt(void);
	CClass*		GetLong(void);
	CClass*		GetUByte(void);
	CClass*		GetUShort(void);
	CClass*		GetUInt(void);
	CClass*		GetULong(void);
	CClass*		GetFloat(void);
	CClass*		GetDouble(void);
	CClass*		GetBool(void);
	CClass*		GetVoid(void);
	CClass*		GetChar(void);
	CClass*		GetWidechar(void);

protected:
	template<class M>
	CClass*		AddClass(EPrimitiveType eFieldType);
	template<class M>
	CClass*		AddClassWithConstructorAndIO(EPrimitiveType eFieldType);
	CClass*		AddClass(const char* szClassName, size uiClassSize, EPrimitiveType eFieldType, size uiFieldOffset);
	CClass*		AddVoid(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
CClass* CPrimitiveClasses::AddClass(EPrimitiveType eFieldType)
{
	M		cInstance;
	size	uiOffset;

	//The value's offset in the primitive object.
	uiOffset = (char*)(&cInstance.mVal) - (char*)(&cInstance);
	return AddClass(M::StaticClassName(), sizeof(M), eFieldType, uiOffset);
}


#endif // __PRIMITIVE_CLASSES_H__

