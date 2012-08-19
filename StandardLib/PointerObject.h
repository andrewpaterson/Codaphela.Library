/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __POINTER_OBJECT_H__
#define __POINTER_OBJECT_H__
#include "Unknown.h"


//This class is roughly a void* (or possibly an Object*).  It should probably have been called CObjectPointer but it's easier to start typing CPo...


class CBaseObject;
class CObject;
class CPointerObject
{
friend class CObjects;
friend class CArrayCommonObject;
friend class CSet;
friend class CArray;
template<class M>
friend class CPointer;
friend class CObjectDeserialiser;

protected:
	CBaseObject*	mpcObject;
	CObject*		mpcEmbedding;  //Collections do not embed pointer objects.  They manage their own pointers.

public:
					CPointerObject();
	void 			Init(CObject* pcEmbedding);
	void			operator = (CBaseObject* ptr);
	void			operator = (CPointerObject pcPointer);
	CBaseObject*	operator -> ();
	CBaseObject*	operator & ();
	BOOL			operator ! ();
	BOOL			IsNotNull(void);
	BOOL			IsNull(void);

	CPointerObject*	This(void);

protected:
	BOOL			Dehollow(void);
	void			PointTo(CBaseObject* pcObject);

	void			Clear(void);
};


#endif // __POINTER_OBJECT_H__

