/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WorldLib

Codaphela WorldLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WorldLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WorldLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __INTERPOLATOR_H__
#define __INTERPOLATOR_H__


template<class M>
class CInterpolator
{
public:
	M		msPosition1;
	M		msPosition2;
	M		msOutput;
	float	mfExponential;

	float	GetExponentialAdjustedPos(float fPos);
	float	GetTrigonometricAdjustedPos(float fPos);
	void	SetExponential(float fExp);
	void	SetOutput(M* psOutput);
	void	SetEndpoints(M* psStart, M* psEnd);
	void	PrivateInit(void);
	void	Interpolate(float fPos, M* psPosition1, M* psPosition2);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
float CInterpolator<M>::GetExponentialAdjustedPos(float fPos)
{
	float	f1;
	float	f2;
	float	f3;

	f1 = powf(fPos, mfExponential);
	f2 = powf(1 - fPos, mfExponential);
	f3 = f1 + f2;
	f3 = 1.0f/f3;
	return f1 * f3;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
float CInterpolator<M>::GetTrigonometricAdjustedPos(float fPos)
{
	float	f1;

	f1 = sinf((fPos * PI) + (PI/2.0f));
	f1 = (1.0f - ((f1 + 1.0f) / 2.0f));
	return f1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CInterpolator<M>::SetExponential(float fExp)
{
	mfExponential = fExp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CInterpolator<M>::SetOutput(M* psOutput)
{
	msOutput = *psOutput;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CInterpolator<M>::SetEndpoints(M* psStart, M* psEnd)
{
	msPosition1 = *psStart;
	msPosition2 = *psEnd;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CInterpolator<M>::PrivateInit(void)
{
	memset(this, 0, sizeof(CInterpolator) - sizeof(float));
	mfExponential = 1.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CInterpolator<M>::Interpolate(float fPos, M* psPosition1, M* psPosition2)
{
    memset(&msOutput, 0, sizeof(M));
}

#endif //__INTERPOLATOR_H__

