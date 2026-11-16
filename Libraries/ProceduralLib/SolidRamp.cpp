/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela ProceduralLib

Codaphela ProceduralLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela ProceduralLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela ProceduralLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"
#include "SolidRamp.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidRamp::Init(Ptr<CSolidTexture> pInput)
{
	PreInit();
	CSolidOperator::Init();
	mpInput = pInput;
	masKeys.Init();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidRamp::Free(void)
{
	masKeys.Kill();
	CSolidOperator::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidRamp::Class(void)
{
	CSolidOperator::Class();
	M_Pointer(mpInput);
	U_Unknown(CArraySolidRampKey, masKeys);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSolidRamp::Save(CObjectWriter* pcFile)
{
	ReturnOnFalse(CSolidOperator::Save(pcFile));
	return masKeys.Write(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSolidRamp::Load(CObjectReader* pcFile)
{
	ReturnOnFalse(CSolidOperator::Load(pcFile));
	return masKeys.Read(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidRamp::AddKey(float32 fValue, ARGB32 uiColour)
{
	SSolidSample	sSample;

	sSample.Init(uiColour);
	AddKey(fValue, &sSample);
}


//////////////////////////////////////////////////////////////////////////
//
// After every key whose value is no more than its own.
//
//////////////////////////////////////////////////////////////////////////
void CSolidRamp::AddKey(float32 fValue, SSolidSample* psSample)
{
	SSolidRampKey*	psKey;
	size			uiIndex;

	uiIndex = masKeys.NumElements();
	while ((uiIndex > 0) && (masKeys.Get(uiIndex - 1)->fValue > fValue))
	{
		uiIndex--;
	}
	psKey = masKeys.InsertAt(uiIndex);
	psKey->fValue = fValue;
	psKey->sSample = *psSample;
}


//////////////////////////////////////////////////////////////////////////
//
// The first key whose value is past fValue, and the key before it: the
// sample is theirs blended by how far fValue is between them.
//
//////////////////////////////////////////////////////////////////////////
void CSolidRamp::At(float32 fValue, SSolidSample* psSample)
{
	SSolidRampKey*	psBefore;
	SSolidRampKey*	psAfter;
	size			uiAfter;

	if (masKeys.NumElements() == 0)
	{
		psSample->Init();
		return;
	}

	uiAfter = 0;
	while ((uiAfter < masKeys.NumElements()) && (masKeys.Get(uiAfter)->fValue <= fValue))
	{
		uiAfter++;
	}
	if (uiAfter == 0)
	{
		*psSample = masKeys.Get(0)->sSample;
		return;
	}

	psBefore = masKeys.Get(uiAfter - 1);
	*psSample = psBefore->sSample;
	if (uiAfter < masKeys.NumElements())
	{
		psAfter = masKeys.Get(uiAfter);
		psSample->Blend(&psAfter->sSample, (fValue - psBefore->fValue) / (psAfter->fValue - psBefore->fValue));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidRamp::TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample)
{
	SSolidSample	sInput;

	Input(mpInput, psPosition, psNormal, fWidth, &sInput);
	At(sInput.Value(), psSample);
}

