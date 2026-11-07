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
#ifndef __PRIMITIVE_H__
#define __PRIMITIVE_H__
#include "BaseLib/FileReader.h"
#include "BaseLib/FileWriter.h"
#include "PrimitiveObject.h"


template<class PRIMITIVE_TYPE>
class CPrimitive : public CPrimitiveObject
{
friend class CPrimitiveClasses;
protected:
	MSVC_ALIGN(PRIMITIVE_TYPE) PRIMITIVE_TYPE mVal;

public:
	CPrimitive(EPrimitiveType eType) : CPrimitiveObject(eType) {}

	void Assign(PRIMITIVE_TYPE val)
	{
		LOG_PRIMITIVE_ASSIGN_DEBUG();

		//We need to only set the containing object dirty if the new value differs from the old one to make primitive assignment consistent with pointers.
		if (mVal != val)
		{
			mVal = val;
			SetDirty();
		}
	}

	operator PRIMITIVE_TYPE() const
	{
		LOG_PRIMITIVE_ACCESS_DEBUG();

		return mVal;
	}

	PRIMITIVE_TYPE* GetPrimitivePointer(void)
	{
		return &mVal;
	}
};


#define PRIMITIVE_TYPE_DEFINITION(CLASS_TYPE, C_PRIMITIVE, DATA_TYPE, WRITE_FUNC, READ_FUNC)  \
class CLASS_TYPE : public CPrimitive<C_PRIMITIVE> \
{ \
CONSTRUCTABLE(CLASS_TYPE) \
public: \
	CLASS_TYPE() : CPrimitive(DATA_TYPE) {}\
 \
	CLASS_TYPE& operator= (C_PRIMITIVE val) \
	{ \
		Assign(val); \
		return *this; \
	} \
\
	bool Save(CFileWriter* pcFile) \
	{ \
		return pcFile-> WRITE_FUNC (mVal); \
	} \
\
	bool Load(CFileReader* pcFile) \
	{ \
		return pcFile-> READ_FUNC (&mVal); \
	} \
};


#endif // __PRIMITIVE_H__

