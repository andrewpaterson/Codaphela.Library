#include "BaseLib/ConstructorCall.h"
#include "GerberVectorShapes.h"
#include "GerberVectoriser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectoriser::Init(CGerberVectorImage* pcImage)
{
	mpcImage = pcImage;
	mpcTarget = pcImage;
	mapcTargets.Init();
	macBlocks.Init();
	masShapes.Init();
	mcStepAndRepeat.Init();
	mpcStepAndRepeat = NULL;
	mpcRegion = NULL;
	mbError = false;
	mszError.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectoriser::Kill(void)
{
	size					i;
	SGerberVectorBlock*		psBlock;

	mszError.Kill();
	mcStepAndRepeat.Kill();
	for (i = 0; i < masShapes.NumElements(); i++)
	{
		masShapes.Get(i)->cShape.Kill();
	}
	masShapes.Kill();
	for (i = 0; i < macBlocks.NumElements(); i++)
	{
		psBlock = (SGerberVectorBlock*)macBlocks.Get(i);
		psBlock->cObjects.Kill();
	}
	macBlocks.Kill();
	mapcTargets.Kill();
	mpcTarget = NULL;
	mpcImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberVectoriser::HasError(void)
{
	return mbError;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CGerberVectoriser::GetError(void)
{
	return mszError.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectoriser::SetError(const char* szError)
{
	if (!mbError)
	{
		mbError = true;
		mszError.Set(szError);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 CGerberVectoriser::GetUnit(CGerberGraphicsState* pcState)
{
	return (pcState->meUnit == GMM_Inches) ? 25.4 : 1.0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectoriser::ToMillimetres(CGerberGraphicsState* pcState, SGerberPoint* psPoint, SFloat64Vec2* psMillimetres)
{
	float64	fUnit;

	fUnit = GetUnit(pcState);
	psMillimetres->Init(psPoint->cX.DoubleValue() * fUnit, psPoint->cY.DoubleValue() * fUnit);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectoriser::GetTransform(CGerberGraphicsState* pcState, SFloat64Vec2* psFlash, SGerberVectorTransform* psTransform)
{
	psTransform->Init();
	psTransform->bMirrorX = (pcState->meMirroring == GLM_X) || (pcState->meMirroring == GLM_XY);
	psTransform->bMirrorY = (pcState->meMirroring == GLM_Y) || (pcState->meMirroring == GLM_XY);
	psTransform->fRotation = pcState->mcRotation.DoubleValue();
	psTransform->fScale = pcState->mcScaling.DoubleValue();
	psTransform->sTranslation = *psFlash;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 CGerberVectoriser::GetStrokeRadius(CGerberGraphicsState* pcState)
{
	//Draws and arcs are stroked with a solid circle; LS scales it.  4.9.1
	return pcState->mpcCurrentAperture->GetParameter(0)->DoubleValue() * GetUnit(pcState) * pcState->mcScaling.DoubleValue() / 2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectoriser::PushTarget(CGerberVectorObjects* pcTarget)
{
	mapcTargets.Add(mpcTarget);
	mpcTarget = pcTarget;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectoriser::PopTarget(void)
{
	mpcTarget = *mapcTargets.Get(mapcTargets.NumElements() - 1);
	mapcTargets.RemoveAt(mapcTargets.NumElements() - 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectoriser::Move(CGerberGraphicsState* pcState, SGerberPoint* psPoint)
{
	SFloat64Vec2	sPoint;

	//Outside a region a move creates nothing.  Inside it starts a contour.
	if (pcState->mbInRegion && mpcRegion)
	{
		ToMillimetres(pcState, psPoint, &sPoint);
		mpcRegion->MoveTo(&sPoint);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectoriser::Draw(CGerberGraphicsState* pcState, SGerberPoint* psStart, SGerberPoint* psEnd)
{
	SFloat64Vec2			sStart;
	SFloat64Vec2			sEnd;
	CGerberVectorObject*	pcObject;

	ToMillimetres(pcState, psStart, &sStart);
	ToMillimetres(pcState, psEnd, &sEnd);
	if (pcState->mbInRegion)
	{
		if (mpcRegion)
		{
			mpcRegion->LineTo(&sEnd);
		}
		return;
	}

	pcObject = mpcTarget->Add(pcState->mePolarity);
	pcObject->BeginPart(true);
	AddGerberStroke(pcObject, &sStart, &sEnd, GetStrokeRadius(pcState));
	pcObject->CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectoriser::Arc(CGerberGraphicsState* pcState, SGerberPoint* psStart, SGerberPoint* psEnd, SGerberPoint* psCenter, bool bClockwise)
{
	SFloat64Vec2			sStart;
	SFloat64Vec2			sEnd;
	SFloat64Vec2			sCenter;
	CGerberVectorObject*	pcObject;

	ToMillimetres(pcState, psStart, &sStart);
	ToMillimetres(pcState, psEnd, &sEnd);
	ToMillimetres(pcState, psCenter, &sCenter);
	if (pcState->mbInRegion)
	{
		if (mpcRegion)
		{
			mpcRegion->ArcTo(&sEnd, &sCenter, bClockwise);
		}
		return;
	}

	pcObject = mpcTarget->Add(pcState->mePolarity);
	pcObject->BeginPart(true);
	AddGerberArcStroke(pcObject, &sStart, &sEnd, &sCenter, bClockwise, GetStrokeRadius(pcState));
	pcObject->CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberVectorObject* CGerberVectoriser::GetApertureShape(CGerberGraphicsState* pcState, CGerberAperture* pcAperture)
{
	size						i;
	SGerberVectorApertureShape*	psShape;

	for (i = 0; i < masShapes.NumElements(); i++)
	{
		psShape = masShapes.Get(i);
		if (psShape->pcAperture == pcAperture)
		{
			return &psShape->cShape;
		}
	}

	psShape = masShapes.Add();
	psShape->pcAperture = pcAperture;
	psShape->cShape.Init(GP_Dark);
	if (!AddGerberApertureShape(&psShape->cShape, pcAperture, GetUnit(pcState)))
	{
		SetError("Could not make the shape of an aperture.");
	}
	return &psShape->cShape;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SGerberVectorBlock* CGerberVectoriser::GetBlock(uint32 uiDCode)
{
	size					i;
	SGerberVectorBlock*		psBlock;

	for (i = 0; i < macBlocks.NumElements(); i++)
	{
		psBlock = (SGerberVectorBlock*)macBlocks.Get(i);
		if (psBlock->uiDCode == uiDCode)
		{
			return psBlock;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectoriser::Flash(CGerberGraphicsState* pcState, SGerberPoint* psPoint)
{
	SFloat64Vec2			sFlash;
	SGerberVectorTransform	sTransform;
	CGerberVectorObject*	pcObject;
	SGerberVectorBlock*		psBlock;
	size					i;
	EGerberPolarity			ePolarity;

	ToMillimetres(pcState, psPoint, &sFlash);
	GetTransform(pcState, &sFlash, &sTransform);

	if (pcState->mpcCurrentAperture->IsType(GAT_Block))
	{
		//The block's objects; a clear flash toggles their polarity.  The block origin is the file origin.  4.11
		psBlock = GetBlock(pcState->mpcCurrentAperture->GetDCode());
		if (psBlock == NULL)
		{
			SetError("Block aperture was not vectorised.");
			return;
		}
		for (i = 0; i < psBlock->cObjects.NumObjects(); i++)
		{
			pcObject = mpcTarget->Add(psBlock->cObjects.GetObject(i));
			ePolarity = pcObject->GetPolarity();
			if (pcState->mePolarity == GP_Clear)
			{
				ePolarity = (ePolarity == GP_Clear) ? GP_Dark : GP_Clear;
			}
			pcObject->SetPolarity(ePolarity);
			pcObject->Transform(&sTransform);
		}
		return;
	}

	pcObject = mpcTarget->Add(GetApertureShape(pcState, pcState->mpcCurrentAperture));
	pcObject->SetPolarity(pcState->mePolarity);
	pcObject->Transform(&sTransform);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectoriser::BeginRegion(CGerberGraphicsState* pcState)
{
	mpcRegion = mpcTarget->Add(pcState->mePolarity);
	mpcRegion->BeginPart(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectoriser::EndRegion(CGerberGraphicsState* pcState)
{
	if (mpcRegion)
	{
		mpcRegion->CalculateBounds();
	}
	mpcRegion = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectoriser::BeginBlock(CGerberGraphicsState* pcState, CGerberCommandApertureBlock* pcBlock)
{
	SGerberVectorBlock*		psBlock;

	psBlock = (SGerberVectorBlock*)macBlocks.Add(sizeof(SGerberVectorBlock));
	psBlock->uiDCode = pcBlock->GetDCode();
	psBlock->cObjects.Init();
	PushTarget(&psBlock->cObjects);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectoriser::EndBlock(CGerberGraphicsState* pcState, CGerberAperture* pcAperture)
{
	PopTarget();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectoriser::BeginStepAndRepeat(CGerberGraphicsState* pcState, CGerberCommandStepAndRepeat* pcStepAndRepeat)
{
	mpcStepAndRepeat = pcStepAndRepeat;
	mcStepAndRepeat.Kill();
	mcStepAndRepeat.Init();
	PushTarget(&mcStepAndRepeat);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectoriser::EndStepAndRepeat(CGerberGraphicsState* pcState)
{
	uint32					x;
	uint32					y;
	size					i;
	float64					fUnit;
	SGerberVectorTransform	sTransform;
	CGerberVectorObject*	pcObject;

	PopTarget();

	//The objects are copied, stepping the whole block first in Y and then in X.  4.12
	fUnit = GetUnit(pcState);
	for (x = 0; x < mpcStepAndRepeat->GetXRepeats(); x++)
	{
		for (y = 0; y < mpcStepAndRepeat->GetYRepeats(); y++)
		{
			sTransform.Init();
			sTransform.sTranslation.Init(x * mpcStepAndRepeat->GetXStep()->DoubleValue() * fUnit, y * mpcStepAndRepeat->GetYStep()->DoubleValue() * fUnit);
			for (i = 0; i < mcStepAndRepeat.NumObjects(); i++)
			{
				pcObject = mpcTarget->Add(mcStepAndRepeat.GetObject(i));
				pcObject->Transform(&sTransform);
			}
		}
	}
	mcStepAndRepeat.Kill();
	mcStepAndRepeat.Init();
	mpcStepAndRepeat = NULL;
}

