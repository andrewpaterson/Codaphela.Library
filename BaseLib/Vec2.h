#pragma once
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __VEC2_H__
#define __VEC2_H__
#include "Define.h"
#include "Constructable.h"
#include "ChunkFile.h"
#include "IntegerHelper.h"
#include "FloatHelper.h"


template<class M, class N>
class SVec2
{
public:
	M x;
	M y;

public:
						SVec2() {};
						SVec2(const M*);
						SVec2(const SVec2<M, N>& v);
						SVec2(M x, M y);

			SVec2<M, N>&	operator += (const SVec2<M, N>&);
			SVec2<M, N>&	operator -= (const SVec2<M, N>&);
			SVec2<M, N>&	operator *= (M);
			SVec2<M, N>&	operator /= (M);

			SVec2<M, N>	operator + () const;
			SVec2<M, N>	operator - () const;

			SVec2<M, N>	operator + (const SVec2<M, N>&) const;
			SVec2<M, N>	operator - (const SVec2<M, N>&) const;
			SVec2<M, N>	operator * (M) const;
			SVec2<M, N>	operator / (M) const;

	friend	SVec2<M, N>	operator * (M, const SVec2<M, N>&);

			bool		operator == (const SVec2<M, N>&) const;
			bool		operator != (const SVec2<M, N>&) const;

			void		Init(M x, M y);
			void		Zero(void);

			bool		Save(CFileWriter* pcFileWriter);
			bool		Load(CFileReader* pcFileReader);

			void 		Copy(const M* ps);

			void		Dump(void);

			void		Add(M* ps);
			N	 		SquareMagnitude(void);
			M			Magnitude(void);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec2<M, N>::SVec2(const M* pf)
{
	x = pf[0];
	y = pf[1];
}

template<class M, class N>
SVec2<M, N>::SVec2(const SVec2<M, N>& v)
{
	x = v.x;
	y = v.y;
}

template<class M, class N>
SVec2<M, N>::SVec2(M fx, M fy)
{
	x = fx;
	y = fy;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec2<M, N>& SVec2<M, N>::operator += (const SVec2<M, N>& v)
{
	x += v.x;
	y += v.y;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec2<M, N>& SVec2<M, N>::operator -= (const SVec2<M, N>& v)
{
	x -= v.x;
	y -= v.y;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec2<M, N>& SVec2<M, N>::operator *= (M f)
{
	x *= f;
	y *= f;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec2<M, N>& SVec2<M, N>::operator /= (M f)
{
	M fInv = 1.0f / f;
	x *= fInv;
	y *= fInv;
	return *this;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec2<M, N> SVec2<M, N>::operator + () const
{
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec2<M, N> SVec2<M, N>::operator - () const
{
	return SVec2(-x, -y);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec2<M, N> SVec2<M, N>::operator + (const SVec2<M, N>& v) const
{
	return SVec2(x + v.x, y + v.y);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec2<M, N> SVec2<M, N>::operator - (const SVec2<M, N>& v) const
{
	return SVec2(x - v.x, y - v.y);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec2<M, N> SVec2<M, N>::operator * (M f) const
{
	return SVec2(x * f, y * f);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec2<M, N> SVec2<M, N>::operator / (M f) const
{
	M fInv = 1.0f / f;
	return SVec2(x * fInv, y * fInv);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec2<M, N> operator * (M f, const SVec2<M, N>& v)
{
	return SVec2<M, N>(f * v.x, f * v.y);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
bool SVec2<M, N>::operator == (const SVec2<M, N>& v) const
{
	return x == v.x && y == v.y;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
bool SVec2<M, N>::operator != (const SVec2<M, N>& v) const
{
	return x != v.x || y != v.y;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
void SVec2<M, N>::Init(M x, M y)
{
	this->x = x;
	this->y = y;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
void SVec2<M, N>::Zero(void)
{
	x = 0.0f;
	y = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
bool SVec2<M, N>::Save(CFileWriter* pcFileWriter)
{
	bool	bResult;

	bResult = pcFileWriter->WritePrimitive<M>(x);
	bResult &= pcFileWriter->WritePrimitive<M>(y);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
bool SVec2<M, N>::Load(CFileReader* pcFileReader)
{
	bool	bResult;

	bResult = pcFileReader->ReadFloat(&x);
	bResult &= pcFileReader->ReadFloat(&y);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>void SVec2<M, N>::Copy(const M* ps)
{
	x = ps->x;
	y = ps->y;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
void SVec2<M, N>::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
void SVec2<M, N>::Add(M* ps)
{
	x += ps->x;
	y += ps->y;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
N SVec2<M, N>::SquareMagnitude(void)
{
	return (N)x * (N)x + (N)y * (N)y;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
M SVec2<M, N>::Magnitude(void)
{
	return (M)SquareRoot((N)SquareMagnitude());
}


#endif // __VEC2_H__

