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
#ifndef __FIXED_POINT_H__
#define __FIXED_POINT_H__
#include "Define.h"
#include "PrimitiveTypes.h"


//A fixed point number is an integer holding a value multiplied by 2^iFractionBits (8 fraction bits hold 1.5 as 384).
//Everything is integer arithmetic, so results are the same in every build and on every compiler, unlike floating
//point whose last bits can change with optimisation.  Rounding is to the nearest, halves away from zero, unless a
//function says otherwise.  Intermediate results are 64 bit; results that don't fit are undefined.


int64	DivideRound(int64 iNumerator, int64 iDenominator);
int64	DivideFloor(int64 iNumerator, int64 iDenominator);
int64	DivideCeiling(int64 iNumerator, int64 iDenominator);

int32	FixedFromFloat(float32 f, int iFractionBits);
int32	FloatToIntScaled(float32 f, int32 iScale);


//A fixed point number whose format is part of its type, so that formats can't be mixed by mistake.  M holds the value
//and N is what a multiply widens to, as in SVec2, SVec3 and SVec4.  Values are only made and converted by name.
//
//Multiplying never loses bits: the result is an N holding the fractions of both operands, which must be the same
//width.  Dividing by another fixed point number takes its fraction bits away.  To changes format rounding to the nearest
//and ToFloor rounding down.
template<class M, class N, int BITS>
class SFixed
{
public:
	typedef M	Storage;
	typedef N	Wide;
	enum { Bits = BITS };

	M	miRaw;

	static SFixed	FromRaw(M iRaw);
	static SFixed	FromInt(int32 i);
	static SFixed	FromFloat(float32 f);

	float32			ToFloat(void) const;
	float64			ToDouble(void) const;
	int32			Floor(void) const;
	int32			Ceiling(void) const;
	int32			Round(void) const;
	SFixed			Fraction(void) const;

	SFixed			operator - (void) const;
	SFixed			operator + (SFixed s) const;
	SFixed			operator - (SFixed s) const;
	SFixed&			operator += (SFixed s);
	SFixed&			operator -= (SFixed s);

	bool			operator == (SFixed s) const;
	bool			operator != (SFixed s) const;
	bool			operator < (SFixed s) const;
	bool			operator <= (SFixed s) const;
	bool			operator > (SFixed s) const;
	bool			operator >= (SFixed s) const;

	template<int OTHER>
	SFixed<N, N, BITS + OTHER>	operator * (SFixed<M, N, OTHER> s) const;

	template<class P, int OTHER>
	SFixed<N, N, BITS - OTHER>	DivideRound(SFixed<P, N, OTHER> s) const;
	template<class P, int OTHER>
	SFixed<N, N, BITS - OTHER>	DivideFloor(SFixed<P, N, OTHER> s) const;

	template<class F>
	F				To(void) const;
	template<class F>
	F				ToFloor(void) const;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
SFixed<M, N, BITS> SFixed<M, N, BITS>::FromRaw(M iRaw)
{
	SFixed	s;

	s.miRaw = iRaw;
	return s;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
SFixed<M, N, BITS> SFixed<M, N, BITS>::FromInt(int32 i)
{
	return FromRaw((M)((int64)i * ((int64)1 << BITS)));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
SFixed<M, N, BITS> SFixed<M, N, BITS>::FromFloat(float32 f)
{
	return FromRaw((M)FixedFromFloat(f, BITS));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
float32 SFixed<M, N, BITS>::ToFloat(void) const
{
	return (float32)ToDouble();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
float64 SFixed<M, N, BITS>::ToDouble(void) const
{
	return (float64)miRaw / (float64)((int64)1 << BITS);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
int32 SFixed<M, N, BITS>::Floor(void) const
{
	return (int32)(miRaw >> BITS);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
int32 SFixed<M, N, BITS>::Ceiling(void) const
{
	return (int32)::DivideCeiling(miRaw, (int64)1 << BITS);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
int32 SFixed<M, N, BITS>::Round(void) const
{
	return (int32)::DivideRound(miRaw, (int64)1 << BITS);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
SFixed<M, N, BITS> SFixed<M, N, BITS>::Fraction(void) const
{
	return *this - FromInt(Floor());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
SFixed<M, N, BITS> SFixed<M, N, BITS>::operator - (void) const
{
	return FromRaw((M)-miRaw);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
SFixed<M, N, BITS> SFixed<M, N, BITS>::operator + (SFixed s) const
{
	return FromRaw((M)(miRaw + s.miRaw));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
SFixed<M, N, BITS> SFixed<M, N, BITS>::operator - (SFixed s) const
{
	return FromRaw((M)(miRaw - s.miRaw));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
SFixed<M, N, BITS>& SFixed<M, N, BITS>::operator += (SFixed s)
{
	miRaw = (M)(miRaw + s.miRaw);
	return *this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
SFixed<M, N, BITS>& SFixed<M, N, BITS>::operator -= (SFixed s)
{
	miRaw = (M)(miRaw - s.miRaw);
	return *this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
bool SFixed<M, N, BITS>::operator == (SFixed s) const
{
	return miRaw == s.miRaw;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
bool SFixed<M, N, BITS>::operator != (SFixed s) const
{
	return miRaw != s.miRaw;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
bool SFixed<M, N, BITS>::operator < (SFixed s) const
{
	return miRaw < s.miRaw;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
bool SFixed<M, N, BITS>::operator <= (SFixed s) const
{
	return miRaw <= s.miRaw;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
bool SFixed<M, N, BITS>::operator > (SFixed s) const
{
	return miRaw > s.miRaw;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
bool SFixed<M, N, BITS>::operator >= (SFixed s) const
{
	return miRaw >= s.miRaw;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
template<int OTHER>
SFixed<N, N, BITS + OTHER> SFixed<M, N, BITS>::operator * (SFixed<M, N, OTHER> s) const
{
	return SFixed<N, N, BITS + OTHER>::FromRaw((N)miRaw * (N)s.miRaw);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
template<class P, int OTHER>
SFixed<N, N, BITS - OTHER> SFixed<M, N, BITS>::DivideRound(SFixed<P, N, OTHER> s) const
{
	return SFixed<N, N, BITS - OTHER>::FromRaw((N)::DivideRound(miRaw, s.miRaw));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
template<class P, int OTHER>
SFixed<N, N, BITS - OTHER> SFixed<M, N, BITS>::DivideFloor(SFixed<P, N, OTHER> s) const
{
	return SFixed<N, N, BITS - OTHER>::FromRaw((N)::DivideFloor(miRaw, s.miRaw));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
template<class F>
F SFixed<M, N, BITS>::To(void) const
{
	if ((int)F::Bits >= BITS)
	{
		return F::FromRaw((typename F::Storage)((int64)miRaw * ((int64)1 << (((int)F::Bits >= BITS) ? (int)F::Bits - BITS : 0))));
	}
	return F::FromRaw((typename F::Storage)::DivideRound(miRaw, (int64)1 << ((BITS >= (int)F::Bits) ? BITS - (int)F::Bits : 0)));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N, int BITS>
template<class F>
F SFixed<M, N, BITS>::ToFloor(void) const
{
	if ((int)F::Bits >= BITS)
	{
		return F::FromRaw((typename F::Storage)((int64)miRaw * ((int64)1 << (((int)F::Bits >= BITS) ? (int)F::Bits - BITS : 0))));
	}
	return F::FromRaw((typename F::Storage)((int64)miRaw >> ((BITS >= (int)F::Bits) ? BITS - (int)F::Bits : 0)));
}


#endif // __FIXED_POINT_H__
