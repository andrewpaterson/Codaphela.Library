/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#ifndef __BITMAP_PACKER_H__
#define __BITMAP_PACKER_H__
#include "BaseLib/ArrayUint32.h"
#include "BaseLib/ArrayInt.h"


//Rows of bits: a shape, or the space shapes are packed into.  miWidth by miHeight, each row miWords 32 bit words, x's bit
//(1 << (x % 32)) in word x / 32.
class CBitRows
{
public:
	CArrayLargeUint32	mauiWords;
	int					miWidth;
	int					miHeight;
	int					miWords;

	void	Init(int iWidth, int iHeight);
	void	Kill(void);

	void	Set(int x, int y);
	bool	Get(int x, int y);
	void	SetHeight(int iHeight);  //Rows added empty, or taken away.
	uint32	GetWord(int y, int iBit);  //The 32 bits from x = iBit, 0 past the edges.
	int		Count(void);
	int		CountRow(int y);

	void	Grow(CBitRows* pcDest);  //Every bit set and those beside it (across and diagonally), a bit wider all round.
	void	Turn(CBitRows* pcDest);  //A quarter turn anticlockwise: (x, y) to (height - 1 - y, x).
};


//Shapes packed into a space of a fixed width, each where it first fits, lowest and then leftmost, without touching those
//there (a raster packer, as xatlas's).  The space grows taller as it needs to.  How many bits each of its rows has clear is
//kept, so a place where a shape's rows have more bits set than the space's rows have clear isn't looked at.
class CBitmapPacker
{
public:
	CBitRows		mcSpace;
	CArrayLargeInt	maiClear;

	void	Init(int iWidth);
	void	Kill(void);

	bool	Fits(CBitRows* pcShape, int x, int y);
	bool	Find(CBitRows* pcShape, int* px, int* py, int iMostY = -1);  //False for a shape wider than the space, or with no place up to iMostY (-1 for anywhere).
	void	Add(CBitRows* pcShape, int x, int y);
	int		GetHeight(void) const;
};


#endif // __BITMAP_PACKER_H__

