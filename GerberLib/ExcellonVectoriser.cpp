#include "GerberVectorShapes.h"
#include "ExcellonVectoriser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonVectoriser::Init(CGerberVectorImage* pcImage, bool bInches)
{
	mpcImage = pcImage;
	mfUnit = bInches ? 25.4 : 1.0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonVectoriser::Kill(void)
{
	mpcImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonVectoriser::Hole(CExcellonCommandToolDefine* pcTool, SGerberPoint* psPoint)
{
	CGerberVectorObject*	pcObject;
	SFloat64Vec2			sCenter;

	sCenter.Init(psPoint->cX.DoubleValue() * mfUnit, psPoint->cY.DoubleValue() * mfUnit);
	pcObject = mpcImage->Add(GP_Dark);
	pcObject->BeginPart(true);
	pcObject->AddCircle(&sCenter, pcTool->GetDiameter()->DoubleValue() * mfUnit / 2, false);
	pcObject->CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonVectoriser::Slot(CExcellonCommandToolDefine* pcTool, SGerberPoint* psStart, SGerberPoint* psEnd)
{
	CGerberVectorObject*	pcObject;
	SFloat64Vec2			sStart;
	SFloat64Vec2			sEnd;

	sStart.Init(psStart->cX.DoubleValue() * mfUnit, psStart->cY.DoubleValue() * mfUnit);
	sEnd.Init(psEnd->cX.DoubleValue() * mfUnit, psEnd->cY.DoubleValue() * mfUnit);
	pcObject = mpcImage->Add(GP_Dark);
	pcObject->BeginPart(true);
	AddVectorStroke(pcObject, &sStart, &sEnd, pcTool->GetDiameter()->DoubleValue() * mfUnit / 2);
	pcObject->CalculateBounds();
}

