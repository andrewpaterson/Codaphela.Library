/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __RECTANGLE_H__
#define __RECTANGLE_H__
#include "BaseLib/DataTypes.h"


//An axis aligned rectangle of V (a 2D vector of M).  N is twice as wide as M so the area can't overflow, the same
//as SVec2's N.  Concrete rectangles name the vector type, eg: CInt32Rect, CFloat64Rect.
//Left 0 to Right 1 gives a width of 1.  0 to 0 gives a width of 0.
template<class V, class M, class N>
class CRectangle
{
public:
	V	msTopLeft;
	V	msBottomRight;

public:
	void	Init(void);
	void 	Init(M iLeft, M iTop, M iRight, M iBottom);
	void	Init(M iWidth, M iHeight);
	void 	Init(V sTopLeft, V sBottomRight);
	void 	Init(V sSize);
	void 	Init(CRectangle<V, M, N>* pcSource);
	void 	Kill(void);
	void	Copy(CRectangle<V, M, N>* pcSource);
	M		GetLeft(void);
	M		GetTop(void);
	M		GetRight(void);
	M		GetBottom(void);
	M 		GetWidth(void);
	M 		GetHeight(void);
	N 		GetArea(void);
	void	SetWidth(M iWidth);
	void	SetHeight(M iHeight);
	void	SetSize(M iWidth, M iHeight);
	void	SetPos(M x, M y);
	void	Shrink(M iShrinkLeft, M iShrinkTop, M iShrinkRight, M iShrinkBottom);
	void	ClipTo(CRectangle<V, M, N>* pcBounds);
	bool	IsValid(void);
	bool	IsIn(M x, M y);
	bool	IsIn(M iLeft, M iTop, M iRight, M iBottom);
	bool	Equals(CRectangle<V, M, N>* pcOther);
	bool	Intersect(CRectangle<V, M, N>* pcOther);
	bool	Inside(CRectangle<V, M, N>* pcOther);
	bool	Outside(CRectangle<V, M, N>* pcOther);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
void CRectangle<V, M, N>::Init(void)
{
	msTopLeft.x = 0;
	msBottomRight.x = 0;
	msTopLeft.y = 0;
	msBottomRight.y = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
void CRectangle<V, M, N>::Init(M iLeft, M iTop, M iRight, M iBottom)
{
	msTopLeft.x = iLeft;
	msTopLeft.y = iTop;
	msBottomRight.x = iRight;
	msBottomRight.y = iBottom;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
void CRectangle<V, M, N>::Init(M iWidth, M iHeight)
{
	msTopLeft.x = 0;
	msTopLeft.y = 0;
	msBottomRight.x = iWidth;
	msBottomRight.y = iHeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
void CRectangle<V, M, N>::Init(V sTopLeft, V sBottomRight)
{
	msTopLeft = sTopLeft;
	msBottomRight = sBottomRight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
void CRectangle<V, M, N>::Init(V sSize)
{
	msTopLeft.x = 0;
	msTopLeft.y = 0;
	msBottomRight.x = sSize.x;
	msBottomRight.y = sSize.y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
void CRectangle<V, M, N>::Init(CRectangle<V, M, N>* pcSource)
{
	Copy(pcSource);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
void CRectangle<V, M, N>::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
void CRectangle<V, M, N>::Copy(CRectangle<V, M, N>* pcSource)
{
	msTopLeft = pcSource->msTopLeft;
	msBottomRight = pcSource->msBottomRight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
M CRectangle<V, M, N>::GetLeft(void)
{
	return msTopLeft.x;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
M CRectangle<V, M, N>::GetTop(void)
{
	return msTopLeft.y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
M CRectangle<V, M, N>::GetRight(void)
{
	return msBottomRight.x;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
M CRectangle<V, M, N>::GetBottom(void)
{
	return msBottomRight.y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
M CRectangle<V, M, N>::GetWidth(void)
{
	return msBottomRight.x - msTopLeft.x;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
M CRectangle<V, M, N>::GetHeight(void)
{
	return msBottomRight.y - msTopLeft.y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
N CRectangle<V, M, N>::GetArea(void)
{
	return (N)GetWidth() * (N)GetHeight();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
void CRectangle<V, M, N>::SetWidth(M iWidth)
{
	msBottomRight.x = msTopLeft.x + iWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
void CRectangle<V, M, N>::SetHeight(M iHeight)
{
	msBottomRight.y = msTopLeft.y + iHeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
void CRectangle<V, M, N>::SetSize(M iWidth, M iHeight)
{
	SetWidth(iWidth);
	SetHeight(iHeight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
void CRectangle<V, M, N>::SetPos(M x, M y)
{
	M	iWidth;
	M	iHeight;

	iWidth = GetWidth();
	iHeight = GetHeight();
	msTopLeft.x = x;
	msTopLeft.y = y;
	msBottomRight.x = msTopLeft.x + iWidth;
	msBottomRight.y = msTopLeft.y + iHeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
void CRectangle<V, M, N>::Shrink(M iShrinkLeft, M iShrinkTop, M iShrinkRight, M iShrinkBottom)
{
	msTopLeft.x += iShrinkLeft;
	msTopLeft.y += iShrinkTop;
	msBottomRight.x -= iShrinkRight;
	msBottomRight.y -= iShrinkBottom;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
void CRectangle<V, M, N>::ClipTo(CRectangle<V, M, N>* pcBounds)
{
	//Keeps only the part of this rectangle inside pcBounds.
	if (msTopLeft.x < pcBounds->msTopLeft.x)
	{
		msTopLeft.x = pcBounds->msTopLeft.x;
	}
	if (msTopLeft.y < pcBounds->msTopLeft.y)
	{
		msTopLeft.y = pcBounds->msTopLeft.y;
	}
	if (msBottomRight.x > pcBounds->msBottomRight.x)
	{
		msBottomRight.x = pcBounds->msBottomRight.x;
	}
	if (msBottomRight.y > pcBounds->msBottomRight.y)
	{
		msBottomRight.y = pcBounds->msBottomRight.y;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
bool CRectangle<V, M, N>::IsValid(void)
{
	return (msTopLeft.x <= msBottomRight.x) && (msTopLeft.y <= msBottomRight.y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
bool CRectangle<V, M, N>::IsIn(M x, M y)
{
	return (((x >= msTopLeft.x) && (x <= msBottomRight.x)) && ((y >= msTopLeft.y) && (y <= msBottomRight.y)));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
bool CRectangle<V, M, N>::IsIn(M iLeft, M iTop, M iRight, M iBottom)
{
	return (IsIn(iLeft, iTop) && IsIn(iRight, iBottom));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
bool CRectangle<V, M, N>::Equals(CRectangle<V, M, N>* pcOther)
{
	if (pcOther)
	{
		if ((msTopLeft.y == pcOther->msTopLeft.y) &&
			(msBottomRight.y == pcOther->msBottomRight.y) &&
			(msTopLeft.x == pcOther->msTopLeft.x) &&
			(msBottomRight.x == pcOther->msBottomRight.x))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
bool CRectangle<V, M, N>::Intersect(CRectangle<V, M, N>* pcOther)
{
	// Check if this intersects Other.
	return !(msBottomRight.x  <= pcOther->msTopLeft.x  ||
			 msTopLeft.x   >= pcOther->msBottomRight.x ||
			 msBottomRight.y <= pcOther->msTopLeft.y   ||
			 msTopLeft.y    >= pcOther->msBottomRight.y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
bool CRectangle<V, M, N>::Inside(CRectangle<V, M, N>* pcOther)
{
	// Check if this is completely inside Other.
	return	(msTopLeft.x	  >= pcOther->msTopLeft.x  &&
			 msBottomRight.x  <= pcOther->msBottomRight.x &&
			 msTopLeft.y	  >= pcOther->msTopLeft.y   &&
			 msBottomRight.y <= pcOther->msBottomRight.y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class M, class N>
bool CRectangle<V, M, N>::Outside(CRectangle<V, M, N>* pcOther)
{
	// Check if this is completely outside Other.
	return (msBottomRight.x  <= pcOther->msTopLeft.x  ||
			msTopLeft.x	 >= pcOther->msBottomRight.x ||
			msBottomRight.y <= pcOther->msTopLeft.y   ||
			msTopLeft.y	 >= pcOther->msBottomRight.y);
}


#endif // __RECTANGLE_H__

