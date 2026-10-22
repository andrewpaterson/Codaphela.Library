#include <math.h>
#include <stdio.h>
#include "BaseLib/Logger.h"
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "GerberPanelEdgeCuts.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelEdgeCuts::Init(void)
{
	mapcLoops.Init();
	mfTolerance = 0.01;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelEdgeCuts::Kill(void)
{
	size				i;
	CGerberPanelLoop*	pcLoop;

	for (i = 0; i < mapcLoops.NumElements(); i++)
	{
		pcLoop = mapcLoops.GetPtr(i);
		SafeKill(pcLoop);
	}
	mapcLoops.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberPanelEdgeCuts::Calculate(CGerberPanelOutlines* pcOutlines, CGerberPanelTabs* pcTabs)
{
	size				i;
	size				j;
	CGerberPanelLoop*	pcSource;
	CGerberPanelLoop*	pcLoop;

	//A copy of every board's outlines, then each tab joins two of them.
	for (i = 0; i < pcOutlines->NumBoards(); i++)
	{
		for (j = 0; j < pcOutlines->NumBoardLoops(i); j++)
		{
			pcSource = pcOutlines->GetBoardLoop(i, j);
			pcLoop = NewMalloc<CGerberPanelLoop>();
			pcLoop->Init();
			pcLoop->masSegments.Copy(&pcSource->masSegments);
			mapcLoops.Add(pcLoop);
		}
	}

	for (i = 0; i < pcTabs->NumTabs(); i++)
	{
		if (!ApplyTab(pcTabs->GetTab(i)))
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
bool CGerberPanelEdgeCuts::Increases(SGerberPanelSegment* psLine, bool bAcrossX)
{
	return bAcrossX ? (psLine->sEnd.y > psLine->sStart.y) : (psLine->sEnd.x > psLine->sStart.x);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelEdgeCuts::PointAt(SGerberPanelSegment* psLine, bool bAcrossX, float64 fAlong, SFloat64Vec2* psPoint)
{
	float64	fT;

	//On the line itself, which may lean by a few nanometres.
	if (bAcrossX)
	{
		fT = (fAlong - psLine->sStart.y) / (psLine->sEnd.y - psLine->sStart.y);
		psPoint->Init(psLine->sStart.x + fT * (psLine->sEnd.x - psLine->sStart.x), fAlong);
	}
	else
	{
		fT = (fAlong - psLine->sStart.x) / (psLine->sEnd.x - psLine->sStart.x);
		psPoint->Init(fAlong, psLine->sStart.y + fT * (psLine->sEnd.y - psLine->sStart.y));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberPanelEdgeCuts::FindEdgeSegment(bool bAcrossX, float64 fEdge, float64 fStart, float64 fEnd, size* puiLoop, size* puiSegment)
{
	size					i;
	size					j;
	CGerberPanelLoop*		pcLoop;
	SGerberPanelSegment*	psSegment;
	float64					fFrom;
	float64					fTo;
	float64					fMin;
	float64					fMax;

	for (i = 0; i < mapcLoops.NumElements(); i++)
	{
		pcLoop = mapcLoops.GetPtr(i);
		for (j = 0; j < pcLoop->masSegments.NumElements(); j++)
		{
			psSegment = pcLoop->masSegments.Get(j);
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
				fFrom = psSegment->sStart.y;
				fTo = psSegment->sEnd.y;
			}
			else
			{
				if ((fabs(psSegment->sStart.y - fEdge) > mfTolerance) || (fabs(psSegment->sEnd.y - fEdge) > mfTolerance))
				{
					continue;
				}
				fFrom = psSegment->sStart.x;
				fTo = psSegment->sEnd.x;
			}
			fMin = (fFrom < fTo) ? fFrom : fTo;
			fMax = (fFrom < fTo) ? fTo : fFrom;
			if ((fMin <= fStart + 0.000001) && (fMax >= fEnd - 0.000001))
			{
				*puiLoop = i;
				*puiSegment = j;
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelEdgeCuts::AddLine(CGerberPanelLoop* pcLoop, SFloat64Vec2* psStart, SFloat64Vec2* psEnd)
{
	SGerberPanelSegment*	psSegment;

	if (hypot(psEnd->x - psStart->x, psEnd->y - psStart->y) < 0.000001)
	{
		return;
	}
	psSegment = pcLoop->masSegments.Add();
	psSegment->bArc = false;
	psSegment->bClockwise = false;
	psSegment->sStart = *psStart;
	psSegment->sEnd = *psEnd;
	psSegment->sCenter.Init(0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelEdgeCuts::AddSegments(CGerberPanelLoop* pcLoop, CGerberPanelLoop* pcSource, size uiFrom, size uiTo)
{
	size	uiNum;
	size	i;

	//uiFrom up to but not including uiTo, round the end of the loop if need be.
	uiNum = pcSource->masSegments.NumElements();
	for (i = uiFrom % uiNum; i != uiTo % uiNum; i = (i + 1) % uiNum)
	{
		*pcLoop->masSegments.Add() = *pcSource->masSegments.Get(i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberPanelEdgeCuts::ApplyTab(SGerberPanelTab* psTab)
{
	size					uiLoopA;
	size					uiLoopB;
	size					uiSegmentA;
	size					uiSegmentB;
	CGerberPanelLoop*		pcLoopA;
	CGerberPanelLoop*		pcLoopB;
	CGerberPanelLoop*		pcJoined;
	CGerberPanelLoop*		pcSplit;
	SGerberPanelSegment		sA;
	SGerberPanelSegment		sB;
	SFloat64Vec2			sP;
	SFloat64Vec2			sQ;
	SFloat64Vec2			sP2;
	SFloat64Vec2			sQ2;
	float64					fPAlong;
	float64					fQAlong;
	size					uiNumA;

	if (!FindEdgeSegment(psTab->bAcrossX, psTab->fEdgeA, psTab->fStart, psTab->fEnd, &uiLoopA, &uiSegmentA) ||
		!FindEdgeSegment(psTab->bAcrossX, psTab->fEdgeB, psTab->fStart, psTab->fEnd, &uiLoopB, &uiSegmentB))
	{
		gcLogger.Error2(__METHOD__, " A tab's edges are not straight lines in the edge cuts.", NULL);
		return false;
	}
	pcLoopA = mapcLoops.GetPtr(uiLoopA);
	pcLoopB = mapcLoops.GetPtr(uiLoopB);
	sA = *pcLoopA->masSegments.Get(uiSegmentA);
	sB = *pcLoopB->masSegments.Get(uiSegmentB);

	//P is where A's edge reaches the tab and Q where it leaves; B's edge reaches Q' (across from Q) first, then P'.
	if (Increases(&sA, psTab->bAcrossX))
	{
		fPAlong = psTab->fStart;
		fQAlong = psTab->fEnd;
	}
	else
	{
		fPAlong = psTab->fEnd;
		fQAlong = psTab->fStart;
	}
	if (Increases(&sB, psTab->bAcrossX) != (fQAlong < fPAlong))
	{
		gcLogger.Error2(__METHOD__, " The outlines either side of a tab don't turn the same way.", NULL);
		return false;
	}
	PointAt(&sA, psTab->bAcrossX, fPAlong, &sP);
	PointAt(&sA, psTab->bAcrossX, fQAlong, &sQ);
	PointAt(&sB, psTab->bAcrossX, fPAlong, &sP2);
	PointAt(&sB, psTab->bAcrossX, fQAlong, &sQ2);

	pcJoined = NewMalloc<CGerberPanelLoop>();
	pcJoined->Init();
	if (uiLoopA != uiLoopB)
	{
		//Q round A to P, across to P', round B to Q', back across to Q.
		uiNumA = pcLoopA->masSegments.NumElements();
		AddLine(pcJoined, &sQ, &sA.sEnd);
		AddSegments(pcJoined, pcLoopA, uiSegmentA + 1, uiSegmentA + uiNumA);
		AddLine(pcJoined, &sA.sStart, &sP);
		AddLine(pcJoined, &sP, &sP2);
		AddLine(pcJoined, &sP2, &sB.sEnd);
		AddSegments(pcJoined, pcLoopB, uiSegmentB + 1, uiSegmentB + pcLoopB->masSegments.NumElements());
		AddLine(pcJoined, &sB.sStart, &sQ2);
		AddLine(pcJoined, &sQ2, &sQ);

		mapcLoops.SetPtr(uiLoopA, pcJoined);
		pcLoopA->Kill();
		free(pcLoopA);
		pcLoopB->Kill();
		free(pcLoopB);
		mapcLoops.RemoveAt(uiLoopB, true);
	}
	else
	{
		//The boards are already joined; the tab splits off the channel between it and the other tab.
		pcSplit = NewMalloc<CGerberPanelLoop>();
		pcSplit->Init();

		AddLine(pcJoined, &sQ, &sA.sEnd);
		AddSegments(pcJoined, pcLoopA, uiSegmentA + 1, uiSegmentB);
		AddLine(pcJoined, &sB.sStart, &sQ2);
		AddLine(pcJoined, &sQ2, &sQ);

		AddLine(pcSplit, &sP2, &sB.sEnd);
		AddSegments(pcSplit, pcLoopA, uiSegmentB + 1, uiSegmentA);
		AddLine(pcSplit, &sA.sStart, &sP);
		AddLine(pcSplit, &sP, &sP2);

		mapcLoops.SetPtr(uiLoopA, pcJoined);
		mapcLoops.Add(pcSplit);
		pcLoopA->Kill();
		free(pcLoopA);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelEdgeCuts::WriteCoordinate(CChars* psz, char cAxis, float64 fMillimetres)
{
	char	sz[32];

	//Format 4.6 in millimetres; whole nanometres.
	snprintf(sz, sizeof(sz), "%c%lld", cAxis, (long long)llround(fMillimetres * 1000000.0));
	psz->Append(sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberPanelEdgeCuts::Write(CChars* psz, float64 fLineWidth)
{
	size					i;
	size					j;
	CGerberPanelLoop*		pcLoop;
	SGerberPanelSegment*	psSegment;
	char					sz[64];
	bool					bArcMode;

	psz->Append("%TF.GenerationSoftware,Codaphela,GerberLib*%\n");
	psz->Append("%TF.FileFunction,Profile,NP*%\n");
	psz->Append("%TF.FilePolarity,Positive*%\n");
	psz->Append("%TF.Part,Array*%\n");
	psz->Append("%FSLAX46Y46*%\n");
	psz->Append("%MOMM*%\n");
	psz->Append("%LPD*%\n");
	psz->Append("G01*\n");
	psz->Append("%TA.AperFunction,Profile*%\n");
	snprintf(sz, sizeof(sz), "%%ADD10C,%.6f*%%\n", fLineWidth);
	psz->Append(sz);
	psz->Append("%TD*%\n");
	psz->Append("D10*\n");

	bArcMode = false;
	for (i = 0; i < mapcLoops.NumElements(); i++)
	{
		pcLoop = mapcLoops.GetPtr(i);
		if (!pcLoop->IsClosed(0.000001))
		{
			gcLogger.Error2(__METHOD__, " An edge cuts outline isn't closed.", NULL);
			return false;
		}
		psSegment = pcLoop->masSegments.Get(0);
		WriteCoordinate(psz, 'X', psSegment->sStart.x);
		WriteCoordinate(psz, 'Y', psSegment->sStart.y);
		psz->Append("D02*\n");
		for (j = 0; j < pcLoop->masSegments.NumElements(); j++)
		{
			psSegment = pcLoop->masSegments.Get(j);
			if (psSegment->bArc)
			{
				if (!bArcMode)
				{
					psz->Append("G75*\n");
				}
				psz->Append(psSegment->bClockwise ? "G02*\n" : "G03*\n");
				bArcMode = true;
				WriteCoordinate(psz, 'X', psSegment->sEnd.x);
				WriteCoordinate(psz, 'Y', psSegment->sEnd.y);
				WriteCoordinate(psz, 'I', psSegment->sCenter.x - psSegment->sStart.x);
				WriteCoordinate(psz, 'J', psSegment->sCenter.y - psSegment->sStart.y);
				psz->Append("D01*\n");
			}
			else
			{
				if (bArcMode)
				{
					psz->Append("G01*\n");
					bArcMode = false;
				}
				WriteCoordinate(psz, 'X', psSegment->sEnd.x);
				WriteCoordinate(psz, 'Y', psSegment->sEnd.y);
				psz->Append("D01*\n");
			}
		}
		if (bArcMode)
		{
			psz->Append("G01*\n");
			bArcMode = false;
		}
	}
	psz->Append("M02*\n");
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CGerberPanelEdgeCuts::NumLoops(void) {return mapcLoops.NumElements();}
CGerberPanelLoop* CGerberPanelEdgeCuts::GetLoop(size uiIndex) {return mapcLoops.SafeGetPtr(uiIndex);}

