#include <math.h>
#include "BaseLib/Logger.h"
#include "BaseLib/ArrayInt.h"
#include "GerberInterpreter.h"
#include "GerberVectoriser.h"
#include "GerberPanelTabs.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelTabs::Init(float64 fTabWidth, float64 fHoleDiameter, float64 fHolePitch, float64 fHoleOffset, float64 fTabSpacing, float64 fCopperClearance)
{
	masTabs.Init();
	mfTabWidth = fTabWidth;
	mfHoleDiameter = fHoleDiameter;
	mfHolePitch = fHolePitch;
	mfHoleOffset = fHoleOffset;
	mfTabSpacing = fTabSpacing;
	mfCopperClearance = fCopperClearance;
	mfTolerance = 0.01;
	mbCopper = false;
	muiMoved = 0;
	muiDropped = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelTabs::Kill(void)
{
	if (mbCopper)
	{
		mcCopper.Kill();
	}
	masTabs.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberPanelTabs::Calculate(CGerberPanel* pcPanel, CGerberPanelOutlines* pcOutlines)
{
	size	i;
	size	j;

	masTabs.ReInit();
	muiMoved = 0;
	muiDropped = 0;
	if (!DrawCopper(pcPanel))
	{
		return false;
	}

	for (i = 0; i < pcOutlines->NumBoards(); i++)
	{
		for (j = 0; j < pcOutlines->NumBoards(); j++)
		{
			if (i != j)
			{
				//Board j to the right of board i, then above it.
				FindTabs(pcOutlines, i, j, true, pcPanel->GetGap());
				FindTabs(pcOutlines, i, j, false, pcPanel->GetGap());
			}
		}
	}
	return CheckJoined(pcPanel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelTabs::FindTabs(CGerberPanelOutlines* pcOutlines, size uiBoardA, size uiBoardB, bool bAcrossX, float64 fGap)
{
	CFloat64Rect		cA;
	CFloat64Rect		cB;
	float64				fDistance;
	float64				fEdgeA;
	float64				fEdgeB;
	float64				fOverlapStart;
	float64				fOverlapEnd;
	CArrayFloat64Vec2	asRangesA;
	CArrayFloat64Vec2	asRangesB;
	CArrayFloat64Vec2	asStraights;
	size				i;
	size				j;
	float64				fStart;
	float64				fEnd;

	pcOutlines->GetOuterLoop(uiBoardA)->GetBounds(&cA);
	pcOutlines->GetOuterLoop(uiBoardB)->GetBounds(&cB);

	//A's right (or top) edge faces B's left (or bottom) edge across about one router gap.
	if (bAcrossX)
	{
		fEdgeA = cA.msBottomRight.x;
		fEdgeB = cB.msTopLeft.x;
		fOverlapStart = (cA.msTopLeft.y > cB.msTopLeft.y) ? cA.msTopLeft.y : cB.msTopLeft.y;
		fOverlapEnd = (cA.msBottomRight.y < cB.msBottomRight.y) ? cA.msBottomRight.y : cB.msBottomRight.y;
	}
	else
	{
		fEdgeA = cA.msBottomRight.y;
		fEdgeB = cB.msTopLeft.y;
		fOverlapStart = (cA.msTopLeft.x > cB.msTopLeft.x) ? cA.msTopLeft.x : cB.msTopLeft.x;
		fOverlapEnd = (cA.msBottomRight.x < cB.msBottomRight.x) ? cA.msBottomRight.x : cB.msBottomRight.x;
	}
	fDistance = fEdgeB - fEdgeA;
	if ((fDistance < fGap - mfTolerance) || (fDistance > fGap + 5 * mfTolerance) || (fOverlapEnd - fOverlapStart < mfTabWidth))
	{
		return;
	}

	//Where both edges are straight.
	asRangesA.Init();
	asRangesB.Init();
	asStraights.Init();
	GetStraightEdges(pcOutlines->GetOuterLoop(uiBoardA), bAcrossX, fEdgeA, &asRangesA);
	GetStraightEdges(pcOutlines->GetOuterLoop(uiBoardB), bAcrossX, fEdgeB, &asRangesB);
	for (i = 0; i < asRangesA.NumElements(); i++)
	{
		for (j = 0; j < asRangesB.NumElements(); j++)
		{
			fStart = asRangesA.Get(i)->x;
			fEnd = asRangesA.Get(i)->y;
			if (asRangesB.Get(j)->x > fStart) fStart = asRangesB.Get(j)->x;
			if (asRangesB.Get(j)->y < fEnd) fEnd = asRangesB.Get(j)->y;
			if (fOverlapStart > fStart) fStart = fOverlapStart;
			if (fOverlapEnd < fEnd) fEnd = fOverlapEnd;
			if (fEnd - fStart >= mfTabWidth)
			{
				asStraights.Add()->Init(fStart, fEnd);
			}
		}
	}
	if (asStraights.NumElements() > 0)
	{
		AddTabs(uiBoardA, uiBoardB, bAcrossX, fEdgeA, fEdgeB, fOverlapStart, fOverlapEnd, &asStraights);
	}
	asStraights.Kill();
	asRangesB.Kill();
	asRangesA.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelTabs::GetStraightEdges(CGerberPanelLoop* pcLoop, bool bAcrossX, float64 fEdge, CArrayFloat64Vec2* pasRanges)
{
	size					i;
	SGerberPanelSegment*	psSegment;
	float64					fStart;
	float64					fEnd;
	SFloat64Vec2*			psRange;

	//Lines along the edge, as ranges along it (x is the start, y the end).
	for (i = 0; i < pcLoop->masSegments.NumElements(); i++)
	{
		psSegment = pcLoop->masSegments.Get(i);
		if (psSegment->bArc)
		{
			continue;
		}
		if (bAcrossX)
		{
			if ((fabs(psSegment->sStart.x - fEdge) > mfTolerance) || (fabs(psSegment->sEnd.x - fEdge) > mfTolerance))
			{
				continue;
			}
			fStart = psSegment->sStart.y;
			fEnd = psSegment->sEnd.y;
		}
		else
		{
			if ((fabs(psSegment->sStart.y - fEdge) > mfTolerance) || (fabs(psSegment->sEnd.y - fEdge) > mfTolerance))
			{
				continue;
			}
			fStart = psSegment->sStart.x;
			fEnd = psSegment->sEnd.x;
		}
		psRange = pasRanges->Add();
		psRange->Init((fStart < fEnd) ? fStart : fEnd, (fStart < fEnd) ? fEnd : fStart);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelTabs::AddTabs(size uiBoardA, size uiBoardB, bool bAcrossX, float64 fEdgeA, float64 fEdgeB, float64 fStart, float64 fEnd, CArrayFloat64Vec2* pasStraights)
{
	float64				fLength;
	int					iNumTabs;
	int					iMaxTabs;
	int					i;
	size				j;
	float64				fCentre;
	SGerberPanelTab*	psTab;
	SGerberPanelTab		sTab;
	float64				fLow;
	float64				fShift;
	float64				fStep;
	bool				bFound;
	int					iStep;

	//One per 30mm of the facing edges, spread evenly, as many as the straight stretches can hold.
	fLength = fEnd - fStart;
	iNumTabs = (int)ceil(fLength / mfTabSpacing);
	if (iNumTabs < 1)
	{
		iNumTabs = 1;
	}
	iMaxTabs = 0;
	for (j = 0; j < pasStraights->NumElements(); j++)
	{
		iMaxTabs += (int)floor((pasStraights->Get(j)->y - pasStraights->Get(j)->x) / mfTabWidth);
	}
	if (iNumTabs > iMaxTabs)
	{
		iNumTabs = iMaxTabs;
	}

	sTab.uiBoardA = uiBoardA;
	sTab.uiBoardB = uiBoardB;
	sTab.bAcrossX = bAcrossX;
	sTab.fEdgeA = fEdgeA;
	sTab.fEdgeB = fEdgeB;
	fLow = fStart;
	fStep = 0.1;
	for (i = 0; i < iNumTabs; i++)
	{
		//The nearest place to the even spacing that is straight on both edges, after the previous tab and clear of copper.
		fCentre = fStart + (i + 0.5) * fLength / iNumTabs;
		bFound = false;
		for (iStep = 0; ((iStep + 1) / 2) * fStep <= fLength; iStep++)
		{
			fShift = ((iStep + 1) / 2) * fStep * ((iStep % 2 == 1) ? 1 : -1);
			sTab.fStart = fCentre + fShift - mfTabWidth / 2;
			sTab.fEnd = fCentre + fShift + mfTabWidth / 2;
			if (sTab.fStart < fLow - 0.000001)
			{
				continue;
			}
			if (IsOnStraight(&sTab, pasStraights) && IsClearOfCopper(&sTab))
			{
				bFound = true;
				break;
			}
		}
		if (bFound)
		{
			if (iStep != 0)
			{
				muiMoved++;
			}
			psTab = masTabs.Add();
			*psTab = sTab;
			fLow = sTab.fEnd;
		}
		else
		{
			muiDropped++;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberPanelTabs::IsOnStraight(SGerberPanelTab* psTab, CArrayFloat64Vec2* pasStraights)
{
	size			i;
	SFloat64Vec2*	psStraight;

	for (i = 0; i < pasStraights->NumElements(); i++)
	{
		psStraight = pasStraights->Get(i);
		if ((psTab->fStart >= psStraight->x - 0.000001) && (psTab->fEnd <= psStraight->y + 0.000001))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberPanelTabs::DrawCopper(CGerberPanel* pcPanel)
{
	size					i;
	size					j;
	size					k;
	CGerberPanelPlacement*	pcPlacement;
	CGerberLayerFile*		pcLayer;
	CGerberVectorImage		cBoard;
	CGerberVectorImage		cPanel;
	CGerberVectoriser		cVectoriser;
	CGerberInterpreter		cInterpreter;
	CGerberRasteriser		cRasteriser;
	CGerberVectorObject*	pcObject;
	SFloat64Vec2			sMinimum;
	bool					bResult;

	//Every copper layer of every board in panel coordinates.
	cPanel.Init();
	for (i = 0; i < pcPanel->NumPlacements(); i++)
	{
		pcPlacement = pcPanel->GetPlacement(i);
		for (j = 0; j < pcPlacement->mpcSet->NumLayers(); j++)
		{
			pcLayer = pcPlacement->mpcSet->GetLayer(j);
			if (pcLayer->GetLayer() != GBL_Copper)
			{
				continue;
			}
			cBoard.Init();
			cVectoriser.Init(&cBoard);
			cInterpreter.Init(pcLayer->GetCommands(), &cVectoriser);
			bResult = cInterpreter.Interpret();
			cInterpreter.Kill();
			cVectoriser.Kill();
			if (!bResult)
			{
				cBoard.Kill();
				cPanel.Kill();
				gcLogger.Error2(__METHOD__, " Board [", pcPlacement->mpcSet->GetName(), "]: a copper layer could not be drawn.", NULL);
				return false;
			}
			for (k = 0; k < cBoard.NumObjects(); k++)
			{
				pcObject = cPanel.Add(cBoard.GetObject(k));
				pcObject->Transform(false, false, pcPlacement->mbRotated ? 90.0 : 0.0, 1.0, &pcPlacement->msOffset);
			}
			cBoard.Kill();
		}
	}

	if (mbCopper)
	{
		mcCopper.Kill();
	}
	sMinimum.Init(0, 0);
	mbCopper = mcCopper.Init(&sMinimum, pcPanel->GetSize(), 20.0);
	if (!mbCopper)
	{
		cPanel.Kill();
		return false;
	}
	cRasteriser.Init();
	bResult = cRasteriser.Rasterise(&cPanel, &mcCopper);
	cRasteriser.Kill();
	cPanel.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberPanelTabs::IsClearOfCopper(SGerberPanelTab* psTab)
{
	CArrayFloat64Vec2	asHoles;
	size				i;
	SFloat64Vec2*		psHole;
	float64				fReach;
	float64				fPixels;
	SFloat64Vec2*		psOrigin;
	int					iLeft;
	int					iRight;
	int					iTop;
	int					iBottom;
	int					x;
	int					y;
	float64				fX;
	float64				fY;
	bool				bClear;

	if (!mbCopper)
	{
		return true;
	}

	//No copper pixel may touch a hole grown by the clearance.
	asHoles.Init();
	GetTabHoles(psTab, &asHoles);
	fReach = mfHoleDiameter / 2 + mfCopperClearance;
	fPixels = mcCopper.GetPixelsPerMillimetre();
	psOrigin = mcCopper.GetOrigin();
	bClear = true;
	for (i = 0; (i < asHoles.NumElements()) && bClear; i++)
	{
		psHole = asHoles.Get(i);
		iLeft = (int)floor((psHole->x - fReach - psOrigin->x) * fPixels);
		iRight = (int)floor((psHole->x + fReach - psOrigin->x) * fPixels);
		iTop = (int)floor((psOrigin->y - (psHole->y + fReach)) * fPixels);
		iBottom = (int)floor((psOrigin->y - (psHole->y - fReach)) * fPixels);
		for (y = iTop; (y <= iBottom) && bClear; y++)
		{
			if ((y < 0) || (y >= mcCopper.GetHeight()))
			{
				continue;
			}
			for (x = iLeft; x <= iRight; x++)
			{
				if ((x < 0) || (x >= mcCopper.GetWidth()) || (mcCopper.Get(x, y) == 0))
				{
					continue;
				}

				//The nearest point of the pixel to the hole's centre.
				fX = psHole->x;
				fY = psHole->y;
				if (fX < psOrigin->x + x / fPixels) fX = psOrigin->x + x / fPixels;
				if (fX > psOrigin->x + (x + 1) / fPixels) fX = psOrigin->x + (x + 1) / fPixels;
				if (fY > psOrigin->y - y / fPixels) fY = psOrigin->y - y / fPixels;
				if (fY < psOrigin->y - (y + 1) / fPixels) fY = psOrigin->y - (y + 1) / fPixels;
				if (hypot(fX - psHole->x, fY - psHole->y) < fReach)
				{
					bClear = false;
					break;
				}
			}
		}
	}
	asHoles.Kill();
	return bClear;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberPanelTabs::CheckJoined(CGerberPanel* pcPanel)
{
	CArrayInt			aiGroups;
	size				i;
	size				j;
	SGerberPanelTab*	psTab;
	int					iFrom;
	int					iTo;

	//Every board ends up in board 0's group.
	aiGroups.Init();
	for (i = 0; i < pcPanel->NumPlacements(); i++)
	{
		aiGroups.Add((int)i);
	}
	for (i = 0; i < masTabs.NumElements(); i++)
	{
		psTab = masTabs.Get(i);
		iFrom = aiGroups.GetValue(psTab->uiBoardB);
		iTo = aiGroups.GetValue(psTab->uiBoardA);
		for (j = 0; j < aiGroups.NumElements(); j++)
		{
			if (aiGroups.GetValue(j) == iFrom)
			{
				aiGroups.SetValue(j, iTo);
			}
		}
	}
	for (i = 0; i < aiGroups.NumElements(); i++)
	{
		if (aiGroups.GetValue(i) != aiGroups.GetValue(0))
		{
			gcLogger.Error2(__METHOD__, " Board [", pcPanel->GetPlacement(i)->mpcSet->GetName(), "] isn't joined to the rest of the panel by any tab.", NULL);
			aiGroups.Kill();
			return false;
		}
	}
	aiGroups.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelTabs::GetMouseBites(CArrayFloat64Vec2* pasHoles)
{
	size	i;

	for (i = 0; i < masTabs.NumElements(); i++)
	{
		GetTabHoles(masTabs.Get(i), pasHoles);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelTabs::GetTabHoles(SGerberPanelTab* psTab, CArrayFloat64Vec2* pasHoles)
{
	int		j;
	int		iNumHoles;
	float64	fCentre;
	float64	fAlong;
	float64	fAcrossA;
	float64	fAcrossB;

	//A row of holes across the width of the tab along each board's edge.
	iNumHoles = (int)floor((mfTabWidth - mfHoleDiameter) / mfHolePitch) + 1;
	fCentre = (psTab->fStart + psTab->fEnd) / 2;
	fAcrossA = psTab->fEdgeA - mfHoleOffset;
	fAcrossB = psTab->fEdgeB + mfHoleOffset;
	for (j = 0; j < iNumHoles; j++)
	{
		fAlong = fCentre + (j - (iNumHoles - 1) / 2.0) * mfHolePitch;
		if (psTab->bAcrossX)
		{
			pasHoles->Add()->Init(fAcrossA, fAlong);
			pasHoles->Add()->Init(fAcrossB, fAlong);
		}
		else
		{
			pasHoles->Add()->Init(fAlong, fAcrossA);
			pasHoles->Add()->Init(fAlong, fAcrossB);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CGerberPanelTabs::NumTabs(void) {return masTabs.NumElements();}
SGerberPanelTab* CGerberPanelTabs::GetTab(size uiIndex) {return masTabs.SafeGet(uiIndex);}
float64 CGerberPanelTabs::GetHoleDiameter(void) {return mfHoleDiameter;}
size CGerberPanelTabs::NumMoved(void) {return muiMoved;}
size CGerberPanelTabs::NumDropped(void) {return muiDropped;}

