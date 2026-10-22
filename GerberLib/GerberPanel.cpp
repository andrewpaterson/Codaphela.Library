#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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
	miOffsetX = 0;
	miOffsetY = 0;
	mcBounds.Init();
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
static void MicrometresToNumber(int32 iMicrometres, CNumber* pcNumber)
{
	char	sz[64];

	snprintf(sz, sizeof(sz), "%s%d.%03d", (iMicrometres < 0) ? "-" : "", abs(iMicrometres) / 1000, abs(iMicrometres) % 1000);
	pcNumber->Init(sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void MillimetresToNumber(float64 fMillimetres, CNumber* pcNumber)
{
	char		sz[64];
	int64		iNanometres;
	uint64		uiNanometres;

	//To the nanometre, which is as fine as the 4.6 format.
	iNanometres = (int64)llround(fMillimetres * 1000000.0);
	uiNanometres = (uint64)((iNanometres < 0) ? -iNanometres : iNanometres);
	snprintf(sz, sizeof(sz), "%s%llu.%06llu", (iNanometres < 0) ? "-" : "", (unsigned long long)(uiNanometres / 1000000), (unsigned long long)(uiNanometres % 1000000));
	pcNumber->Init(sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelPlacement::GetOffset(CNumber* pcX, CNumber* pcY)
{
	MicrometresToNumber(miOffsetX, pcX);
	MicrometresToNumber(miOffsetY, pcY);
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
	CInt32Rect							cRect;
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
		pcPlacement->miOffsetX = pcPacked->msTopLeft.x - iMinimumX;
		pcPlacement->miOffsetY = pcPacked->msTopLeft.y - iMinimumY;
		pcPlacement->msOffset.Init(pcPlacement->miOffsetX / 1000.0, pcPlacement->miOffsetY / 1000.0);
		pcPlacement->mcBounds.Init(pcPacked->msTopLeft.x / 1000.0, pcPacked->msTopLeft.y / 1000.0, pcPacked->msBottomRight.x / 1000.0, pcPacked->msBottomRight.y / 1000.0);
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
	CFloat64Rect*	pcBounds;

	//Rounded outwards.  The nanometre allowance stops floating point noise in exact values adding a micrometre.
	//The outline's msTopLeft is its minimum corner, the bottom left with Gerber's y up.
	pcBounds = pcSet->GetOutlineBounds();
	*piLeft = (int32)floor(pcBounds->msTopLeft.x * 1000.0 + 0.000001);
	*piBottom = (int32)floor(pcBounds->msTopLeft.y * 1000.0 + 0.000001);
	*piRight = (int32)ceil(pcBounds->msBottomRight.x * 1000.0 - 0.000001);
	*piTop = (int32)ceil(pcBounds->msBottomRight.y * 1000.0 - 0.000001);
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
float64 CGerberPanel::GetGap(void) {return mfGap;}
float64 CGerberPanel::GetEdge(void) {return mfEdge;}
size CGerberPanel::NumPlacements(void) {return macPlacements.NumElements();}
CGerberPanelPlacement* CGerberPanel::GetPlacement(size uiIndex) {return macPlacements.SafeGet(uiIndex);}

