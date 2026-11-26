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
#ifndef __CONSTRUCTABLE_H__
#define __CONSTRUCTABLE_H__
#include "PrimitiveTypes.h"


class _CForceVirtualSuper1
{
	virtual void _ForceVirtual1(void) {};
};


class _CForceVirtualSuper2
{
	virtual void _ForceVirtual2(void) {};
};


//Inherit from super 1 and 2 to always force virtual multiple inheritance style function calls.
class CConstructable : public _CForceVirtualSuper1, public _CForceVirtualSuper2
{
public:
	virtual const char*		ClassName(void) =0;
	virtual size			ClassSize(void) =0;
};


#define CLASS_NAME(c) #c


//For a class whose name is itself a macro (the name is expanded before it is turned into a string).
#define CONSTRUCTABLE_MACRO(c) CONSTRUCTABLE(c)


//A CConstructable's name and size: it overrides CConstructable's ClassName and ClassSize.
#define CONSTRUCTABLE(c) CLASS_NAME_FUNCTIONS(c, override)


//The same for a class that isn't a CConstructable (a value type, as CChars and SFloat32Vec3): the functions are its own.
#define NAMED_CLASS(c) CLASS_NAME_FUNCTIONS(c, )


#define CLASS_NAME_FUNCTIONS(c, OVERRIDE) \
public:\
static const char* StaticClassName(void)\
{\
	return #c;\
}\
const char* ClassName(void) OVERRIDE\
{\
	return #c;\
}\
size ClassSize(void) OVERRIDE\
{\
	return sizeof(c);\
}

//  Destructable must be present on every class implementing Kill whose Kill method must be called on destruction.
//  It cannot just be added to, say, CConstructable because due to the 'accuracies' of C++ only the Kill method on
//  CConstructable will be called.  Not the Kill methods on any sub-classes.
#define DESTRUCTABLE(c) \
public:\
~c(void)\
{\
	Kill();\
}


#endif // __CONSTRUCTABLE_H__

