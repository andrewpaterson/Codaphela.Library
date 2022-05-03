/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "NumberControl.h"


CNumberControl gcNumberControl;

char gsz_e[] =  "2.718281828459045235360287471352662497757247093699959574966967627724076630353547594571382178525166427427466391932003059921817413596629043572900334295260595630738132328627943490763233829880753195251019011573834187930702154089149934884167509244761460668082264";
char gsz_pi[] = "3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117067982148086513282306647093844609550582231725359408128481117450284102701938521105559644622948954930381964428810975665933446128475648233786783165271201909145648";


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void NumberInit(int iScratchPadSize)
{
	gcNumberControl.Init(iScratchPadSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void NumberKill(void)
{
	gcNumberControl.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumberControl::Init(int iScratchPadSize)
{
	mcScratchPad.Init(iScratchPadSize);
	mc_pi.Init(gsz_pi);
	mc_e.Init(gsz_e);
	mc_pi_lots.c.Init(gsz_pi, 1, 255);
	mc_e_lots.c.Init(gsz_e, 1, 255);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumberControl::Kill(void)
{
	mcScratchPad.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumberControl::Pi(CNumber* pcDest)
{
	pcDest->Copy(&mc_pi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumberControl::Pi(CNumber* pcDest, int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	pcDest->Copy(&mc_pi_lots.c, cMaxWholeNumbers, cMaxDecimals);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumberControl::E(CNumber* pcDest)
{
	pcDest->Copy(&mc_e);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumberControl::E(CNumber* pcDest, int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	pcDest->Copy(&mc_e_lots.c, cMaxWholeNumbers, cMaxDecimals);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumberControl::Add(int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	return (CNumber*)mcScratchPad.Add(NUMBER_SIZE(cMaxWholeNumbers, cMaxDecimals));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumberControl::Remove(void)
{
	mcScratchPad.Remove();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumberControl::Remove(int iNumToRemove)
{
	mcScratchPad.Remove(iNumToRemove);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CNumberControl::NumElements(void)
{
	return mcScratchPad.NumElements();
}

