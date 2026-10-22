#include <math.h>
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "GerberInterpreter.h"
#include "GerberVectoriser.h"
#include "ExcellonInterpreter.h"
#include "ExcellonVectoriser.h"
#include "GerberPanelOutlines.h"
#include "GerberBoardRenderer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberBoardRenderer::Init(float64 fPixelsPerMillimetre, float64 fMargin)
{
	mfPixelsPerMillimetre = fPixelsPerMillimetre;
	mfMargin = fMargin;

	//A dark bench, tan FR4, bare copper, gold plating, green mask and white silkscreen.
	SetColour(&msBackground, 30, 30, 34);
	SetColour(&msSubstrate, 194, 172, 112);
	SetColour(&msCopper, 184, 115, 51);
	SetColour(&msFinish, 212, 175, 55);
	SetColour(&msMask, 16, 92, 48);
	mfMaskOpacity = 0.82;
	SetColour(&msSilk, 245, 245, 240);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberBoardRenderer::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberBoardRenderer::SetColour(SGerberRenderColour* psColour, uint8 r, uint8 g, uint8 b)
{
	psColour->r = r;
	psColour->g = g;
	psColour->b = b;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberBoardRenderer::Blend(float64* afColour, SGerberRenderColour* psColour, float64 fAmount)
{
	afColour[0] += (psColour->r - afColour[0]) * fAmount;
	afColour[1] += (psColour->g - afColour[1]) * fAmount;
	afColour[2] += (psColour->b - afColour[2]) * fAmount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberBoardRenderer::Rasterise(CGerberVectorImage* pcImage, CGerberRaster* pcRaster)
{
	CGerberRasteriser	cRasteriser;
	bool				bResult;

	cRasteriser.Init();
	bResult = cRasteriser.Rasterise(pcImage, pcRaster);
	cRasteriser.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberBoardRenderer::DrawShape(CGerberFileSet* pcSet, CGerberRaster* pcRaster)
{
	CGerberPanelOutlines	cOutlines;
	CGerberVectorImage		cImage;
	CGerberVectorObject*	pcShape;
	CGerberPanelLoop*		pcLoop;
	SGerberPanelSegment*	psSegment;
	size					i;
	size					j;
	bool					bResult;

	//The outline filled; cut outs and router channels turn the other way so they stay empty.
	cOutlines.Init();
	if (!cOutlines.Read(pcSet))
	{
		cOutlines.Kill();
		return false;
	}
	cImage.Init();
	pcShape = cImage.Add(GP_Dark);
	pcShape->BeginPart(true);
	for (i = 0; i < cOutlines.NumBoardLoops(0); i++)
	{
		pcLoop = cOutlines.GetBoardLoop(0, i);
		pcShape->MoveTo(&pcLoop->masSegments.Get(0)->sStart);
		for (j = 0; j < pcLoop->masSegments.NumElements(); j++)
		{
			psSegment = pcLoop->masSegments.Get(j);
			if (psSegment->bArc)
			{
				pcShape->ArcTo(&psSegment->sEnd, &psSegment->sCenter, psSegment->bClockwise);
			}
			else
			{
				pcShape->LineTo(&psSegment->sEnd);
			}
		}
	}
	pcShape->CalculateBounds();
	cOutlines.Kill();

	bResult = Rasterise(&cImage, pcRaster);
	cImage.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberBoardRenderer::DrawLayer(CGerberFileSet* pcSet, EGerberBoardLayer eLayer, EGerberBoardSide eSide, CGerberRaster* pcRaster)
{
	CGerberLayerFile*	pcLayer;
	CGerberVectorImage	cImage;
	CGerberVectoriser	cVectoriser;
	CGerberInterpreter	cInterpreter;
	bool				bResult;

	pcRaster->Clear();
	pcLayer = pcSet->GetLayer(eLayer, eSide);
	if (pcLayer == NULL)
	{
		return true;
	}

	cImage.Init();
	cVectoriser.Init(&cImage);
	cInterpreter.Init(pcLayer->GetCommands(), &cVectoriser);
	bResult = cInterpreter.Interpret();
	cInterpreter.Kill();
	cVectoriser.Kill();
	if (bResult)
	{
		bResult = Rasterise(&cImage, pcRaster);
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Could not draw [", pcLayer->GetFileName(), "].", NULL);
	}
	cImage.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberBoardRenderer::DrawDrills(CGerberFileSet* pcSet, CGerberRaster* pcRaster)
{
	CGerberVectorImage		cImage;
	CExcellonVectoriser		cVectoriser;
	CExcellonInterpreter	cInterpreter;
	CGerberDrillFile*		pcDrill;
	CExcellonCommands*		pcCommands;
	size					i;
	size					j;
	bool					bInches;
	bool					bResult;

	//Plated and non-plated holes both go through.
	cImage.Init();
	bResult = true;
	for (i = 0; (i < pcSet->NumDrills()) && bResult; i++)
	{
		pcDrill = pcSet->GetDrill(i);
		pcCommands = pcDrill->GetCommands();
		bInches = false;
		for (j = 0; j < pcCommands->NumCommands(); j++)
		{
			if (pcCommands->GetCommand(j)->IsType(EC_Unit))
			{
				bInches = ((CExcellonCommandUnit*)pcCommands->GetCommand(j))->IsInches();
			}
		}
		cVectoriser.Init(&cImage, bInches);
		cInterpreter.Init(pcCommands, &cVectoriser);
		bResult = cInterpreter.Interpret();
		cInterpreter.Kill();
		cVectoriser.Kill();
		if (!bResult)
		{
			gcLogger.Error2(__METHOD__, " Could not draw [", pcDrill->GetFileName(), "].", NULL);
		}
	}
	pcRaster->Clear();
	if (bResult)
	{
		bResult = Rasterise(&cImage, pcRaster);
	}
	cImage.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CGerberBoardRenderer::Render(CGerberFileSet* pcSet, EGerberBoardSide eSide)
{
	CGerberRaster	cShape;
	CGerberRaster	cCopper;
	CGerberRaster	cMask;
	CGerberRaster	cSilk;
	CGerberRaster	cHoles;
	SFloat64Vec2	sMinimum;
	SFloat64Vec2	sMaximum;
	CFloat64Rect*	pcBounds;
	Ptr<CImage>		pcImage;
	uint8*			puiPixels;
	uint8*			puiPixel;
	float64			afColour[3];
	float64			fShape;
	float64			fCopper;
	float64			fOpening;
	int				iWidth;
	int				iHeight;
	int				x;
	int				y;
	int				iSourceX;
	bool			bResult;

	if (!pcSet->HasOutlineBounds())
	{
		gcLogger.Error2(__METHOD__, " Board [", pcSet->GetName(), "] has no edge cuts outline.", NULL);
		return NULL;
	}
	pcBounds = pcSet->GetOutlineBounds();
	sMinimum.Init(pcBounds->msTopLeft.x - mfMargin, pcBounds->msTopLeft.y - mfMargin);
	sMaximum.Init(pcBounds->msBottomRight.x + mfMargin, pcBounds->msBottomRight.y + mfMargin);

	//All the layers on the same pixels.
	bResult = cShape.Init(&sMinimum, &sMaximum, mfPixelsPerMillimetre);
	bResult &= cCopper.Init(&sMinimum, &sMaximum, mfPixelsPerMillimetre);
	bResult &= cMask.Init(&sMinimum, &sMaximum, mfPixelsPerMillimetre);
	bResult &= cSilk.Init(&sMinimum, &sMaximum, mfPixelsPerMillimetre);
	bResult &= cHoles.Init(&sMinimum, &sMaximum, mfPixelsPerMillimetre);
	bResult = bResult && DrawShape(pcSet, &cShape);
	bResult = bResult && DrawLayer(pcSet, GBL_Copper, eSide, &cCopper);
	bResult = bResult && DrawLayer(pcSet, GBL_SolderMask, eSide, &cMask);
	bResult = bResult && DrawLayer(pcSet, GBL_Silk, eSide, &cSilk);
	bResult = bResult && DrawDrills(pcSet, &cHoles);

	pcImage = NULL;
	if (bResult)
	{
		iWidth = cShape.GetWidth();
		iHeight = cShape.GetHeight();
		pcImage = OMalloc<CImage>(iWidth, iHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		puiPixels = (uint8*)pcImage->GetData();
		for (y = 0; y < iHeight; y++)
		{
			for (x = 0; x < iWidth; x++)
			{
				//Seen from the back the board is turned over left to right.
				iSourceX = (eSide == GBS_Bottom) ? iWidth - 1 - x : x;
				fShape = cShape.Get(iSourceX, y) / 255.0;
				fCopper = cCopper.Get(iSourceX, y) / 255.0;
				fOpening = cMask.Get(iSourceX, y) / 255.0;

				afColour[0] = msBackground.r;
				afColour[1] = msBackground.g;
				afColour[2] = msBackground.b;
				Blend(afColour, &msSubstrate, fShape);
				Blend(afColour, &msCopper, fCopper);
				Blend(afColour, &msFinish, fCopper * fOpening);
				Blend(afColour, &msMask, fShape * (1 - fOpening) * mfMaskOpacity);
				Blend(afColour, &msSilk, (cSilk.Get(iSourceX, y) / 255.0) * fShape);
				Blend(afColour, &msBackground, cHoles.Get(iSourceX, y) / 255.0);

				puiPixel = &puiPixels[((size)y * iWidth + x) * 3];
				puiPixel[0] = (uint8)lround(afColour[0]);
				puiPixel[1] = (uint8)lround(afColour[1]);
				puiPixel[2] = (uint8)lround(afColour[2]);
			}
		}
	}

	cHoles.Kill();
	cSilk.Kill();
	cMask.Kill();
	cCopper.Kill();
	cShape.Kill();
	return pcImage;
}

