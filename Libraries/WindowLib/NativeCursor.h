/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __NATIVE_CURSOR_H__
#define __NATIVE_CURSOR_H__


//The pointer's shape over a component (see CComponent::SetCursor), shown by the native window.
enum ENativeCursor
{
	NC_Arrow,
	NC_SizeWestEast,		//A vertical sash, or a window's left or right edge.
	NC_SizeNorthSouth,		//A horizontal sash, or a window's top or bottom edge.
	NC_SizeNorthWestSouthEast,
	NC_SizeNorthEastSouthWest,
	NC_Move,
	NC_NumCursors,
};


#endif // __NATIVE_CURSOR_H__

