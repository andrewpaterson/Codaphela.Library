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
#ifndef __VEC3_H__
#define __VEC3_H__
#include "Define.h"
#include "Constructable.h"
#include "ChunkFile.h"


template<class M, class N>
class SVec3
{
CONSTRUCTABLE(SVec3);
public:
	M x;
	M y;
	M z;

public:
							SVec3() {};
							SVec3<M, N>(const M*);
							SVec3<M, N>(const SVec3<M, N>& v);
							SVec3<M, N>(M x, M y, M z);

			SVec3<M, N>&	operator += (const SVec3<M, N>&);
			SVec3<M, N>&	operator -= (const SVec3<M, N>&);
			SVec3<M, N>&	operator *= (M);
			SVec3<M, N>&	operator /= (M);

			SVec3<M, N>		operator + () const;
			SVec3<M, N>		operator - () const;

			SVec3<M, N>		operator + (const SVec3<M, N>&) const;
			SVec3<M, N>		operator - (const SVec3<M, N>&) const;
			SVec3<M, N>		operator * (M) const;
			SVec3<M, N>		operator / (M) const;

	friend	SVec3<M, N>		operator * (M, const SVec3<M, N>&);

			bool			operator == (const SVec3<M, N>&) const;
			bool			operator != (const SVec3<M, N>&) const;

			void 			Init(M x, M y, M z);
			void 			Init(const SVec3<M, N>& v);
			void			Init(SVec3* pv);
			void 			Zero(void);

			bool 			Save(CFileWriter* pcFileWriter);
			bool 			Load(CFileReader* pcFileReader);

			void			Dump(void);

			void			Add(const SVec3<M, N>& v);
			void			Add(SVec3<M, N>* pv);
			void			Subtract(const SVec3<M, N>& v);
			void			Subtract(SVec3<M, N>* pv);
			M 				SquareMagnitude(void);
			M 				Magnitude(void);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec3<M, N>::SVec3(const M* pf)
{
	x = pf[0];
	y = pf[1];
	z = pf[2];
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec3<M, N>::SVec3(const SVec3<M, N>& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec3<M, N>::SVec3(M x, M y, M z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec3<M, N>& SVec3<M, N>::operator += (const SVec3<M, N>& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec3<M, N>& SVec3<M, N>::operator -= (const SVec3<M, N>& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec3<M, N>& SVec3<M, N>::operator *= (M f)
{
	x *= f;
	y *= f;
	z *= f;
	return *this;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec3<M, N>& SVec3<M, N>::operator /= (M f)
{
	M fInv = 1.0f / f;
	x *= fInv;
	y *= fInv;
	z *= fInv;
	return *this;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec3<M, N> SVec3<M, N>::operator + () const
{
	return *this;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec3<M, N> SVec3<M, N>::operator - () const
{
	return SVec3(-x, -y, -z);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec3<M, N> SVec3<M, N>::operator + (const SVec3<M, N>& v) const
{
	return SVec3(x + v.x, y + v.y, z + v.z);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec3<M, N> SVec3<M, N>::operator - (const SVec3<M, N>& v) const
{
	return SVec3(x - v.x, y - v.y, z - v.z);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec3<M, N> SVec3<M, N>::operator * (M f) const
{
	return SVec3(x * f, y * f, z * f);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec3<M, N> SVec3<M, N>::operator / (M f) const
{
	M fInv = 1.0f / f;
	return SVec3(x * fInv, y * fInv, z * fInv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
SVec3<M, N> operator * (M f, const SVec3<M, N>& v)
{
	return SVec3(f * v.x, f * v.y, f * v.z);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
bool SVec3<M, N>::operator == (const SVec3<M, N>& v) const
{
	return x == v.x && y == v.y && z == v.z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
bool SVec3<M, N>::operator != (const SVec3<M, N>& v) const
{
	return x != v.x || y != v.y || z != v.z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
void SVec3<M, N>::Init(M x, M y, M z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
void SVec3<M, N>::Init(const SVec3<M, N>& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
void SVec3<M, N>::Init(SVec3<M, N>* pv)
{
	x = pv->x;
	y = pv->y;
	z = pv->z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
void SVec3<M, N>::Zero(void)
{
	x = y = z = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
void SVec3<M, N>::Add(const SVec3<M, N>& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
void SVec3<M, N>::Add(SVec3<M, N>* pv)
{
	x += pv->x;
	y += pv->y;
	z += pv->z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
void SVec3<M, N>::Subtract(const SVec3<M, N>& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
void SVec3<M, N>::Subtract(SVec3<M, N>* pv)
{
	x -= pv->x;
	y -= pv->y;
	z -= pv->z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
M SVec3<M, N>::Magnitude(void)
{
	return SquareRoot(SquareMagnitude());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
M SVec3<M, N>::SquareMagnitude(void)
{
	return x*x + y*y + z*z;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
bool SVec3<M, N>::Save(CFileWriter* pcFileWriter)
{
	bool	bResult;

	bResult = pcFileWriter->WritePrimitive<M>(x);
	bResult &= pcFileWriter->WritePrimitive<M>(y);
	bResult &= pcFileWriter->WritePrimitive(z);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
bool SVec3<M, N>::Load(CFileReader* pcFileReader)
{
	bool	bResult;

	bResult = pcFileReader->ReadPrimitive(&x);
	bResult &= pcFileReader->ReadPrimitive(&y);
	bResult &= pcFileReader->ReadPrimitive(&z);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
void SVec3<M, N>::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz, 4);
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}


#endif // __VEC3_H__

