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
#ifndef __VEC4_H__
#define __VEC4_H__
#include "Define.h"
#include "FileWriter.h"
#include "FileReader.h"
#include "Constructable.h"
#include "IntegerHelper.h"
#include "FloatHelper.h"
#include "Vec3.h"


template<class M, class N>
class SVec4
{
public:
	M x;
	M y;
	M z;
	M w;

public:
							SVec4() {};
							SVec4<M, N>(const M*);
							SVec4<M, N>(const SVec4<M, N>& v);
							SVec4<M, N>(const SVec3<M, N>& v, M w);
							SVec4<M, N>(M x, M y, M z, M w);

			SVec4<M, N>&	operator += (const SVec4<M, N>&);
			SVec4<M, N>&	operator -= (const SVec4<M, N>&);
			SVec4<M, N>&	operator *= (M);
			SVec4<M, N>&	operator /= (M);

			SVec4<M, N>		operator + () const;
			SVec4<M, N>		operator - () const;

			SVec4<M, N>		operator + (const SVec4<M, N>&) const;
			SVec4<M, N>		operator - (const SVec4<M, N>&) const;
			SVec4<M, N>		operator * (M) const;
			SVec4<M, N>		operator / (M) const;

	friend	SVec4<M, N>		operator * (M, const SVec4<M, N>&);

			bool			operator == (const SVec4<M, N>&) const;
			bool			operator != (const SVec4<M, N>&) const;	

			void 			Init(M x, M y, M z, M w);
			void 			Init(const SVec4<M, N>& v);
			void			Init(SVec4* pv);
			void 			Zero(void);

			bool 			Save(CFileWriter* pcFileWriter);
			bool 			Load(CFileReader* pcFileReader);

			void			Dump(void);

			void			Add(const SVec4<M, N>& v);
			void			Add(SVec4<M, N>* pv);
			void			Subtract(const SVec4<M, N>& v);
			void			Subtract(SVec4<M, N>* pv);
			N 				SquareMagnitude(void);
			M 				Magnitude(void);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec4<M, N>::SVec4(const M *pf)
{
	x = pf[0];
	y = pf[1];
	z = pf[2];
	w = pf[3];
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec4<M, N>::SVec4(const SVec4<M, N>& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	w = v.w;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec4<M, N>::SVec4(const SVec3<M, N>& v, M w)
{
	x = v.x;
	y = v.y;
	z = v.z;
	w = w;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec4<M, N>::SVec4(M fx, M fy, M fz, M fw)
{
	x = fx;
	y = fy;
	z = fz;
	w = fw;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec4<M, N>& SVec4<M, N>::operator += (const SVec4<M, N>& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec4<M, N>& SVec4<M, N>::operator -= (const SVec4<M, N>& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
	return *this;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec4<M, N>& SVec4<M, N>::operator *= (M f)
{
	x *= f;
	y *= f;
	z *= f;
	w *= f;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec4<M, N>& SVec4<M, N>::operator /= (M f)
{
	M fInv = 1.0f / f;
	x *= fInv;
	y *= fInv;
	z *= fInv;
	w *= fInv;
	return *this;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec4<M, N> SVec4<M, N>::operator + () const
{
	return *this;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec4<M, N> SVec4<M, N>::operator - () const
{
	return SVec4(-x, -y, -z, -w);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec4<M, N> SVec4<M, N>::operator + (const SVec4<M, N>& v) const
{
	return SVec4(x + v.x, y + v.y, z + v.z, w + v.w);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec4<M, N> SVec4<M, N>::operator - (const SVec4<M, N>& v) const
{
	return SVec4(x - v.x, y - v.y, z - v.z, w - v.w);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec4<M, N> SVec4<M, N>::operator * (M f) const
{
	return SVec4(x * f, y * f, z * f, w * f);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec4<M, N> SVec4<M, N>::operator / (M f) const
{
	M fInv = 1.0f / f;
	return SVec4(x * fInv, y * fInv, z * fInv, w * fInv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec4<M, N> operator * (M f, const SVec4<M, N>& v)
{
	return SVec4(f * v.x, f * v.y, f * v.z, f * v.w);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
bool SVec4<M, N>::operator == (const SVec4<M, N>& v) const
{
	return x == v.x && y == v.y && z == v.z && w == v.w;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
bool SVec4<M, N>::operator != (const SVec4<M, N>& v) const
{
	return x != v.x || y != v.y || z != v.z || w != v.w;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
void SVec4<M, N>::Init(M x, M y, M z, M w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
void SVec4<M, N>::Init(const SVec4<M, N>& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	w = v.w;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
void SVec4<M, N>::Init(SVec4* pv)
{
	x = pv->x;
	y = pv->y;
	z = pv->z;
	w = pv->w;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
void SVec4<M, N>::Zero(void)
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
	w = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
bool SVec4<M, N>::Save(CFileWriter* pcFileWriter)
{
	bool	bResult;

	bResult = pcFileWriter->WritePrimitive<M>(x);
	bResult &= pcFileWriter->WritePrimitive<M>(y);
	bResult &= pcFileWriter->WritePrimitive<M>(z);
	bResult &= pcFileWriter->WritePrimitive<M>(w);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
bool SVec4<M, N>::Load(CFileReader* pcFileReader)
{
	bool	bResult;

	bResult = pcFileReader->ReadPrimitive<M>(&x);
	bResult &= pcFileReader->ReadPrimitive<M>(&y);
	bResult &= pcFileReader->ReadPrimitive<M>(&z);
	bResult &= pcFileReader->ReadPrimitive<M>(&w);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
void SVec4<M, N>::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz, 4);
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
void SVec4<M, N>::Add(const SVec4<M, N>& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
void SVec4<M, N>::Add(SVec4<M, N>* pv)
{
	x += pv->x;
	y += pv->y;
	z += pv->z;
	w += pv->w;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
void SVec4<M, N>::Subtract(const SVec4<M, N>& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
void SVec4<M, N>::Subtract(SVec4<M, N>* pv)
{
	x -= pv->x;
	y -= pv->y;
	z -= pv->z;
	w -= pv->w;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
M SVec4<M, N>::Magnitude(void)
{
	return (M)SquareRoot((N)SquareMagnitude());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
N SVec4<M, N>::SquareMagnitude(void)
{
	return (N)x * (N)x + (N)y * (N)y + (N)z * (N)z + (N)w * (N)w;
}


#endif // __VEC4_H__

