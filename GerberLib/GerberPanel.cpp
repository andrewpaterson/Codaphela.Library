#include <math.h>
#include "BaseLib/Logger.h"
#include "SupportLib/RectangleBestPackerWithRotation.h"
#include "GerberPanel.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelPlacement::Init(CGerberFileSet* pcSet)
{
	mpcSet = pcSet;
	mbRotated = false;
	msOffset.Init(0, 0);
	msMinimum.Init(0, 0);
	msMaximum.Init(0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelPlacement::Place(SFloat64Vec2* psPoint, SFloat64Vec2* psDest)
{
	if (mbRotated)
	{
		psDest->Init(-psPoint->y + msOffset.x, psPoint->x + msOffset.y);
	}
	else
	{
		psDest->Init(psPoint->x + msOffset.x, psPoint->y + msOffset.y);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanel::Init(float64 fGap, float64 fEdge, bool bAllowRotation)
{
	mfGap = fGap;
	mfEdge = fEdge;
	mbAllowRotation = bAllowRotation;
	macPlacements.Init();
	msSize.Init(0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanel::Kill(void)
{
	macPlacements.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberPanel::Pack(CGerberFileSets* pcSets)
{
	CRectangleBestPacker*				pcPacker;
	CRectangleBestPacker				cPacker;
	CRectangleBestPackerWithRotation	cRotationPacker;
	CArrayPackedRectangle				acPacked;
	CPackedRectangle*					pcPacked;
	CGerberPanelPlacement*				pcPlacement;
	CGerberFileSet*						pcSet;
	CRectangle							cRect;
	SInt32Vec2							sSize;
	size								i;
	int32								iLeft;
	int32								iBottom;
	int32								iRight;
	int32								iTop;
	int32								iMinimumX;
	int32								iMinimumY;

	macPlacements.ReInit();
	msSize.Init(0, 0);

	if (mbAllowRotation)
	{
		cRotationPacker.Init(ToMicrometres(mfEdge), ToMicrometres(mfGap));
		pcPacker = &cRotationPacker;
	}
	else
	{
		cPacker.Init(ToMicrometres(mfEdge), ToMicrometres(mfGap));
		pcPacker = &cPacker;
	}

	for (i = 0; i < pcSets->NumSets(); i++)
	{
		pcSet = pcSets->Get(i);
		if (!pcSet->HasOutlineBounds())
		{
			gcLogger.Error2(__METHOD__, " Board [", pcSet->GetName(), "] has no edge cuts outline.", NULL);
			pcPacker->Kill();
			return false;
		}
		GetMicrometreBounds(pcSet, &iLeft, &iBottom, &iRight, &iTop);
		cRect.Init(iRight - iLeft, iTop - iBottom);
		pcPacker->AddRectangle(&cRect, pcSet);
	}

	pcPacker->Pack();
	acPacked.Init();
	pcPacker->GetPackedRectangles(&acPacked);
	sSize = pcPacker->GetPackedImageSize();
	pcPacker->Kill();

	if (acPacked.NumElements() != pcSets->NumSets())
	{
		gcLogger.Error2(__METHOD__, " Not every board could be packed.", NULL);
		acPacked.Kill();
		return false;
	}

	//The packer's y runs down but the layout is just as valid read with y up, so packed positions are used as Gerber positions.
	for (i = 0; i < acPacked.NumElements(); i++)
	{
		pcPacked = acPacked.Get(i);
		pcSet = (CGerberFileSet*)pcPacked->GetUserData();
		GetMicrometreBounds(pcSet, &iLeft, &iBottom, &iRight, &iTop);

		pcPlacement = macPlacements.Add();
		pcPlacement->Init(pcSet);
		pcPlacement->mbRotated = pcPacked->IsRotated();
		if (pcPlacement->mbRotated)
		{
			//Turned counterclockwise the outline runs from (-top, left) to (-bottom, right).
			iMinimumX = -iTop;
			iMinimumY = iLeft;
		}
		else
		{
			iMinimumX = iLeft;
			iMinimumY = iBottom;
		}
		pcPlacement->msOffset.Init((pcPacked->msTopLeft.x - iMinimumX) / 1000.0, (pcPacked->msTopLeft.y - iMinimumY) / 1000.0);
		pcPlacement->msMinimum.Init(pcPacked->msTopLeft.x / 1000.0, pcPacked->msTopLeft.y / 1000.0);
		pcPlacement->msMaximum.Init(pcPacked->msBottomRight.x / 1000.0, pcPacked->msBottomRight.y / 1000.0);
	}
	acPacked.Kill();

	msSize.Init(sSize.x / 1000.0, sSize.y / 1000.0);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanel::GetMicrometreBounds(CGerberFileSet* pcSet, int32* piLeft, int32* piBottom, int32* piRight, int32* piTop)
{
	//Rounded outwards.  The nanometre allowance stops floating point noise in exact values adding a micrometre.
	*piLeft = (int32)floor(pcSet->GetOutlineMinimum()->x * 1000.0 + 0.000001);
	*piBottom = (int32)floor(pcSet->GetOutlineMinimum()->y * 1000.0 + 0.000001);
	*piRight = (int32)ceil(pcSet->GetOutlineMaximum()->x * 1000.0 - 0.000001);
	*piTop = (int32)ceil(pcSet->GetOutlineMaximum()->y * 1000.0 - 0.000001);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CGerberPanel::ToMicrometres(float64 fMillimetres)
{
	//Gaps round up.
	return (int32)ceil(fMillimetres * 1000.0 - 0.000001);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberPanelPlacement* CGerberPanel::GetPlacement(CGerberFileSet* pcSet)
{
	size					i;
	CGerberPanelPlacement*	pcPlacement;

	for (i = 0; i < macPlacements.NumElements(); i++)
	{
		pcPlacement = macPlacements.Get(i);
		if (pcPlacement->mpcSet == pcSet)
		{
			return pcPlacement;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat64Vec2* CGerberPanel::GetSize(void) {return &msSize;}
size CGerberPanel::NumPlacements(void) {return macPlacements.NumElements();}
CGerberPanelPlacement* CGerberPanel::GetPlacement(size uiIndex) {return macPlacements.SafeGet(uiIndex);}

