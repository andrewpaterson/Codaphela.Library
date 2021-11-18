/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
	virtual unsigned int	ClassSize(void) =0;
};


#define CONSTRUCTABLE(c) \
public:\
const char* ClassName(void)\
{\
	return #c##;\
}\
unsigned int ClassSize(void)\
{\
	return sizeof(c);\
}

#define DESTRUCTABLE(c) \
public:\
~c(void)\
{\
	Kill();\
}


#endif // !__CONSTRUCTABLE_H__

