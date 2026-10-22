#include <math.h>
#include "BaseLib/Logger.h"
#include "BaseLib/Numbers.h"
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "GerberGraphicsState.h"
#include "GerberInterpreter.h"
#include "GerberPanelOutlines.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ReverseGerberPanelSegment(SGerberPanelSegment* psSegment)
{
	SFloat64Vec2	sTemp;

	sTemp = psSegment->sStart;
	psSegment->sStart = psSegment->sEnd;
	psSegment->sEnd = sTemp;
	psSegment->bClockwise = !psSegment->bClockwise;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void IncludeGerberPanelPoint(float64 x, float64 y, CFloat64Rect* pcBounds, bool* pbFirst)
{
	if (*pbFirst)
	{
		pcBounds->Init(x, y, x, y);
		*pbFirst = false;
		return;
	}
	if (x < pcBounds->msTopLeft.x) pcBounds->msTopLeft.x = x;
	if (y < pcBounds->msTopLeft.y) pcBounds->msTopLeft.y = y;
	if (x > pcBounds->msBottomRight.x) pcBounds->msBottomRight.x = x;
	if (y > pcBounds->msBottomRight.y) pcBounds->msBottomRight.y = y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void IncludeGerberPanelSegmentBounds(SGerberPanelSegment* psSegment, CFloat64Rect* pcBounds, bool* pbFirst)
{
	float64	fRadius;
	float64	fStart;
	float64	fSweep;
	float64	fAngle;
	int		iQuarter;

	IncludeGerberPanelPoint(psSegment->sStart.x, psSegment->sStart.y, pcBounds, pbFirst);
	IncludeGerberPanelPoint(psSegment->sEnd.x, psSegment->sEnd.y, pcBounds, pbFirst);
	if (!psSegment->bArc)
	{
		return;
	}

	//The axis extremes the arc passes through.
	fRadius = hypot(psSegment->sStart.x - psSegment->sCenter.x, psSegment->sStart.y - psSegment->sCenter.y);
	fStart = atan2(psSegment->sStart.y - psSegment->sCenter.y, psSegment->sStart.x - psSegment->sCenter.x);
	fSweep = Float64Vec2ArcSweep(&psSegment->sStart, &psSegment->sEnd, &psSegment->sCenter, psSegment->bClockwise);
	for (iQuarter = 0; iQuarter < 4; iQuarter++)
	{
		fAngle = iQuarter * N_PI_64 / 2 - fStart;
		if (psSegment->bClockwise)
		{
			fAngle = -fAngle;
		}
		while (fAngle < 0)
		{
			fAngle += 2 * N_PI_64;
		}
		while (fAngle >= 2 * N_PI_64)
		{
			fAngle -= 2 * N_PI_64;
		}
		if (fAngle <= fSweep)
		{
			IncludeGerberPanelPoint(psSegment->sCenter.x + fRadius * cos(iQuarter * N_PI_64 / 2), psSegment->sCenter.y + fRadius * sin(iQuarter * N_PI_64 / 2), pcBounds, pbFirst);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelLoop::Init(void)
{
	masSegments.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelLoop::Kill(void)
{
	masSegments.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 CGerberPanelLoop::Area(void)
{
	size					i;
	SGerberPanelSegment*	psSegment;
	float64					fArea;
	float64					fRadius;
	float64					fSweep;

	//The polygon through the ends, plus the circular segment between each arc and its chord.
	fArea = 0;
	for (i = 0; i < masSegments.NumElements(); i++)
	{
		psSegment = masSegments.Get(i);
		fArea += (psSegment->sStart.x * psSegment->sEnd.y - psSegment->sEnd.x * psSegment->sStart.y) / 2;
		if (psSegment->bArc)
		{
			fRadius = hypot(psSegment->sStart.x - psSegment->sCenter.x, psSegment->sStart.y - psSegment->sCenter.y);
			fSweep = Float64Vec2ArcSweep(&psSegment->sStart, &psSegment->sEnd, &psSegment->sCenter, psSegment->bClockwise);
			fArea += (psSegment->bClockwise ? -1 : 1) * fRadius * fRadius * (fSweep - sin(fSweep)) / 2;
		}
	}
	return fArea;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelLoop::Reverse(void)
{
	size					i;
	size					uiNum;
	SGerberPanelSegment		sTemp;

	uiNum = masSegments.NumElements();
	for (i = 0; i < uiNum / 2; i++)
	{
		sTemp = *masSegments.Get(i);
		*masSegments.Get(i) = *masSegments.Get(uiNum - 1 - i);
		*masSegments.Get(uiNum - 1 - i) = sTemp;
	}
	for (i = 0; i < uiNum; i++)
	{
		ReverseGerberPanelSegment(masSegments.Get(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelLoop::GetBounds(CFloat64Rect* pcBounds)
{
	size	i;
	bool	bFirst;

	bFirst = true;
	pcBounds->Init();
	for (i = 0; i < masSegments.NumElements(); i++)
	{
		IncludeGerberPanelSegmentBounds(masSegments.Get(i), pcBounds, &bFirst);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberPanelLoop::IsClosed(float64 fTolerance)
{
	size					i;
	size					uiNum;
	SGerberPanelSegment*	psSegment;
	SGerberPanelSegment*	psNext;

	uiNum = masSegments.NumElements();
	if (uiNum == 0)
	{
		return false;
	}
	for (i = 0; i < uiNum; i++)
	{
		psSegment = masSegments.Get(i);
		psNext = masSegments.Get((i + 1) % uiNum);
		if (hypot(psSegment->sEnd.x - psNext->sStart.x, psSegment->sEnd.y - psNext->sStart.y) > fTolerance)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelOutlines::Init(float64 fTolerance)
{
	mapcLoops.Init();
	masPieces.Init();
	maiOuterLoops.Init();
	maiFirstLoops.Init();
	maiNumLoops.Init();
	mpcPlacement = NULL;
	mpcSet = NULL;
	mfTolerance = fTolerance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelOutlines::Kill(void)
{
	size				i;
	CGerberPanelLoop*	pcLoop;

	for (i = 0; i < mapcLoops.NumElements(); i++)
	{
		pcLoop = mapcLoops.GetPtr(i);
		SafeKill(pcLoop);
	}
	mapcLoops.Kill();
	maiNumLoops.Kill();
	maiFirstLoops.Kill();
	maiOuterLoops.Kill();
	masPieces.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberPanelOutlines::Read(CGerberPanel* pcPanel)
{
	size	i;

	for (i = 0; i < pcPanel->NumPlacements(); i++)
	{
		mpcPlacement = pcPanel->GetPlacement(i);
		if (!ReadBoard(mpcPlacement->mpcSet))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberPanelOutlines::Read(CGerberFileSet* pcSet)
{
	mpcPlacement = NULL;
	return ReadBoard(pcSet);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberPanelOutlines::ReadBoard(CGerberFileSet* pcSet)
{
	CGerberLayerFile*		pcOutline;
	CGerberInterpreter		cInterpreter;
	bool					bResult;

	mpcSet = pcSet;
	pcOutline = pcSet->GetLayer(GBL_Outline, GBS_Both);
	if (pcOutline == NULL)
	{
		return Error("The board has no edge cuts layer.");
	}

	masPieces.ReInit();
	cInterpreter.Init(pcOutline->GetCommands(), this);
	bResult = cInterpreter.Interpret();
	cInterpreter.Kill();
	if (!bResult)
	{
		return Error("The edge cuts layer could not be interpreted.");
	}
	return JoinPieces();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelOutlines::ToPanel(CGerberGraphicsState* pcState, SGerberPoint* psPoint, SFloat64Vec2* psPanel)
{
	SFloat64Vec2	sBoard;
	float64			fUnit;

	fUnit = (pcState->meUnit == GMM_Inches) ? 25.4 : 1.0;
	sBoard.Init(psPoint->cX.DoubleValue() * fUnit, psPoint->cY.DoubleValue() * fUnit);
	if (mpcPlacement)
	{
		mpcPlacement->Place(&sBoard, psPanel);
	}
	else
	{
		*psPanel = sBoard;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelOutlines::Draw(CGerberGraphicsState* pcState, SGerberPoint* psStart, SGerberPoint* psEnd)
{
	SGerberPanelSegment*	psPiece;

	psPiece = masPieces.Add();
	psPiece->bArc = false;
	psPiece->bClockwise = false;
	ToPanel(pcState, psStart, &psPiece->sStart);
	ToPanel(pcState, psEnd, &psPiece->sEnd);
	psPiece->sCenter.Init(0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelOutlines::Arc(CGerberGraphicsState* pcState, SGerberPoint* psStart, SGerberPoint* psEnd, SGerberPoint* psCenter, bool bClockwise)
{
	SGerberPanelSegment*	psPiece;

	//Turning 90 degrees keeps an arc's direction.
	psPiece = masPieces.Add();
	psPiece->bArc = true;
	psPiece->bClockwise = bClockwise;
	ToPanel(pcState, psStart, &psPiece->sStart);
	ToPanel(pcState, psEnd, &psPiece->sEnd);
	ToPanel(pcState, psCenter, &psPiece->sCenter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberPanelOutlines::JoinPieces(void)
{
	CArrayTemplatePrimitive<bool>	abUsed;
	size					i;
	size					j;
	size					uiNumPieces;
	size					uiFirstLoop;
	CGerberPanelLoop*		pcLoop;
	SGerberPanelSegment*	psPiece;
	SGerberPanelSegment*	psLast;
	SGerberPanelSegment*	psFirst;
	SGerberPanelSegment		sNext;
	bool					bFound;
	float64					fArea;
	float64					fLargest;
	int						iOuter;

	uiNumPieces = masPieces.NumElements();
	abUsed.Init();
	for (i = 0; i < uiNumPieces; i++)
	{
		abUsed.Add(false);
	}

	uiFirstLoop = mapcLoops.NumElements();
	for (i = 0; i < uiNumPieces; i++)
	{
		if (abUsed.GetValue(i))
		{
			continue;
		}

		pcLoop = NewMalloc<CGerberPanelLoop>();
		pcLoop->Init();
		mapcLoops.Add(pcLoop);
		*pcLoop->masSegments.Add() = *masPieces.Get(i);
		abUsed.SetValue(i, true);

		//Follow the pieces until back at the start.
		for (;;)
		{
			psLast = pcLoop->masSegments.Tail();
			psFirst = pcLoop->masSegments.Get(0);
			if ((pcLoop->masSegments.NumElements() > 1) && (hypot(psLast->sEnd.x - psFirst->sStart.x, psLast->sEnd.y - psFirst->sStart.y) <= mfTolerance))
			{
				psLast->sEnd = psFirst->sStart;
				break;
			}
			if ((pcLoop->masSegments.NumElements() == 1) && psFirst->bArc && (hypot(psLast->sEnd.x - psFirst->sStart.x, psLast->sEnd.y - psFirst->sStart.y) <= mfTolerance))
			{
				//A whole circle.
				psLast->sEnd = psFirst->sStart;
				break;
			}

			bFound = false;
			for (j = 0; j < uiNumPieces; j++)
			{
				if (abUsed.GetValue(j))
				{
					continue;
				}
				psPiece = masPieces.Get(j);
				sNext = *psPiece;
				if (hypot(psLast->sEnd.x - sNext.sEnd.x, psLast->sEnd.y - sNext.sEnd.y) <= mfTolerance)
				{
					ReverseGerberPanelSegment(&sNext);
				}
				else if (hypot(psLast->sEnd.x - sNext.sStart.x, psLast->sEnd.y - sNext.sStart.y) > mfTolerance)
				{
					continue;
				}
				sNext.sStart = psLast->sEnd;
				*pcLoop->masSegments.Add() = sNext;
				abUsed.SetValue(j, true);
				bFound = true;
				break;
			}
			if (!bFound)
			{
				abUsed.Kill();
				return Error("The edge cuts don't join into closed outlines.");
			}
		}
	}
	abUsed.Kill();

	if (mapcLoops.NumElements() == uiFirstLoop)
	{
		return Error("The edge cuts layer draws nothing.");
	}

	//The biggest loop is the outside; it turns counterclockwise and the cut outs clockwise.  For a panel's edge cuts
	//the cut outs are the router channel pieces.
	iOuter = (int)uiFirstLoop;
	fLargest = -1;
	for (i = uiFirstLoop; i < mapcLoops.NumElements(); i++)
	{
		fArea = fabs(mapcLoops.GetPtr(i)->Area());
		if (fArea > fLargest)
		{
			fLargest = fArea;
			iOuter = (int)i;
		}
	}
	for (i = uiFirstLoop; i < mapcLoops.NumElements(); i++)
	{
		pcLoop = mapcLoops.GetPtr(i);
		fArea = pcLoop->Area();
		if (((int)i == iOuter) ? (fArea < 0) : (fArea > 0))
		{
			pcLoop->Reverse();
		}
	}
	maiOuterLoops.Add(iOuter);
	maiFirstLoops.Add((int)uiFirstLoop);
	maiNumLoops.Add((int)(mapcLoops.NumElements() - uiFirstLoop));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CGerberPanelOutlines::NumBoards(void)
{
	return maiOuterLoops.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberPanelLoop* CGerberPanelOutlines::GetOuterLoop(size uiBoard)
{
	return mapcLoops.GetPtr(maiOuterLoops.GetValue(uiBoard));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CGerberPanelOutlines::NumBoardLoops(size uiBoard)
{
	return maiNumLoops.GetValue(uiBoard);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberPanelLoop* CGerberPanelOutlines::GetBoardLoop(size uiBoard, size uiIndex)
{
	return mapcLoops.GetPtr(maiFirstLoops.GetValue(uiBoard) + uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberPanelOutlines::Error(const char* szError)
{
	if (mpcSet)
	{
		gcLogger.Error2(__METHOD__, " Board [", mpcSet->GetName(), "]: ", szError, NULL);
	}
	else
	{
		gcLogger.Error2(__METHOD__, " ", szError, NULL);
	}
	return false;
}

