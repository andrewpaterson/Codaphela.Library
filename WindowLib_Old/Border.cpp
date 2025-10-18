/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "Border.h"
#include "BorderParameters.h"
#include "Viewport.h"
#include "SupportLib/ColourARGB.h"
#include "ComponentFactory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SInsets::Init(void)
{
	mafWidths[0] = 0;
	mafWidths[1] = 0;
	mafWidths[2] = 0;
	mafWidths[3] = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SInsets::Init(int* pafWidths)
{
	mafWidths[0] = pafWidths[0];
	mafWidths[1] = pafWidths[1];
	mafWidths[2] = pafWidths[2];
	mafWidths[3] = pafWidths[3];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBorder::Init(CViewport* pcViewport)
{
	CDrawable::Init(pcViewport);

	mfAlpha = 1.0f;
	mpcBorderParameters = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBorder::Kill(void)
{
	mpcBorderParameters = NULL;
	CDrawable::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBorder::Draw(void)
{
	CreateQuads();
	return CDrawable::Draw();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBorder::SetBorderParameters(CBorderParameters* pcParameters)
{
	mpcBorderParameters = pcParameters;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBorder::CalculateEdgePositions(int* pafEdgeLength, int* pafEdgeStart)
{
	int			i;
	int			iPow2;
	int			iCornerNum;

	pafEdgeLength[0] = msActualSize.x;
	pafEdgeLength[1] = msActualSize.y;
	pafEdgeLength[2] = msActualSize.x;
	pafEdgeLength[3] = msActualSize.y;
	pafEdgeStart[0] = 0;
	pafEdgeStart[1] = 0;
	pafEdgeStart[2] = 0;
	pafEdgeStart[3] = 0;

	//Setup the corners and work out edge lengths and positions;
	iPow2 = 1;
	for (i = 0; i < BORDER_QUADS; i++)
	{
		if (mpcBorderParameters->meBorderStyle & iPow2)
		{
			if (iPow2 & BORDER_CORNERS)
			{
				iCornerNum = i / 2;
				if (iPow2 & BS_TopLeft)
				{
					pafEdgeLength[0] -= mpcBorderParameters->masCornerSizes[iCornerNum].x;
					pafEdgeLength[3] -= mpcBorderParameters->masCornerSizes[iCornerNum].y;
					pafEdgeStart[0] = mpcBorderParameters->masCornerSizes[iCornerNum].x;
					pafEdgeStart[3] = mpcBorderParameters->masCornerSizes[iCornerNum].y;
				}
				else if (iPow2 & BS_TopRight)
				{
					pafEdgeLength[0] -= mpcBorderParameters->masCornerSizes[iCornerNum].x;
					pafEdgeLength[1] -= mpcBorderParameters->masCornerSizes[iCornerNum].y;
					pafEdgeStart[1] = mpcBorderParameters->masCornerSizes[iCornerNum].y;
				}
				else if (iPow2 & BS_BottomRight)
				{
					pafEdgeLength[1] -= mpcBorderParameters->masCornerSizes[iCornerNum].y;
					pafEdgeLength[2] -= mpcBorderParameters->masCornerSizes[iCornerNum].x;
					pafEdgeStart[2] = mpcBorderParameters->masCornerSizes[iCornerNum].x;
				}
				else if (iPow2 & BS_BottomLeft)
				{
					pafEdgeLength[2] -= mpcBorderParameters->masCornerSizes[iCornerNum].x;
					pafEdgeLength[3] -= mpcBorderParameters->masCornerSizes[iCornerNum].y;
				}
			}
		}
		iPow2 *= 2;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBorder::SetQuadDimensions(CQuad2D* mapcQuads[BORDER_QUADS], int* pafEdgeLength, int* pafEdgeStart)
{
	int			i;
	int			iPow2;
	CQuad2D*	pcQuad;
	int			iEdgeNum;
	int			iCornerNum;

	//Setup the corners and work out edge lengths and positions;
	iPow2 = 1;
	for (i = 0; i < BORDER_QUADS; i++)
	{
		if (mpcBorderParameters->meBorderStyle & iPow2)
		{
			pcQuad = mapcQuads[i];
			if (iPow2 & BORDER_CORNERS)
			{
				iCornerNum = i / 2;
				if (iPow2 & BS_TopLeft)
				{
					pcQuad->SetDimensions((int)mpcBorderParameters->masCornerSizes[iCornerNum].x, (int)mpcBorderParameters->masCornerSizes[iCornerNum].y);
				}
				else if (iPow2 & BS_TopRight)
				{
					pcQuad->SetDimensions((int)mpcBorderParameters->masCornerSizes[iCornerNum].x, (int)mpcBorderParameters->masCornerSizes[iCornerNum].y);
					pcQuad->MovePosition((msActualSize.x - mpcBorderParameters->masCornerSizes[iCornerNum].x), 0);
				}
				else if (iPow2 & BS_BottomRight)
				{
					pcQuad->SetDimensions((int)mpcBorderParameters->masCornerSizes[iCornerNum].x, (int)mpcBorderParameters->masCornerSizes[iCornerNum].y);
					pcQuad->MovePosition((msActualSize.x - mpcBorderParameters->masCornerSizes[iCornerNum].x), (msActualSize.y - mpcBorderParameters->masCornerSizes[iCornerNum].y));
				}
				else if (iPow2 & BS_BottomLeft)
				{
					pcQuad->SetDimensions((int)mpcBorderParameters->masCornerSizes[iCornerNum].x, (int)mpcBorderParameters->masCornerSizes[iCornerNum].y);
					pcQuad->MovePosition(0, (msActualSize.y - mpcBorderParameters->masCornerSizes[iCornerNum].y));
				}
			}

			if (iPow2 & BORDER_EDGES)
			{
				iEdgeNum = (i-1) / 2;
				if (iPow2 & BS_Top)
				{
					pcQuad->SetDimensions((int)pafEdgeLength[0], (int)mpcBorderParameters->maiEdgeWidths[0]);
					pcQuad->MovePosition(pafEdgeStart[0], 0);
				}
				else if (iPow2 & BS_Right)
				{
					pcQuad->SetDimensions((int)mpcBorderParameters->maiEdgeWidths[1], (int)pafEdgeLength[1]);
					pcQuad->MovePosition((msActualSize.x - mpcBorderParameters->maiEdgeWidths[1]), pafEdgeStart[1]);
				}
				else if (iPow2 & BS_Bottom)
				{
					pcQuad->SetDimensions((int)pafEdgeLength[2], (int)mpcBorderParameters->maiEdgeWidths[2]);
					pcQuad->MovePosition(pafEdgeStart[2], (msActualSize.y - mpcBorderParameters->maiEdgeWidths[2]));
				}
				else if (iPow2 & BS_Left)
				{
					pcQuad->SetDimensions((int)mpcBorderParameters->maiEdgeWidths[3], (int)pafEdgeLength[3]);
					pcQuad->MovePosition(0, pafEdgeStart[3]);
				}
			}
		}
		iPow2 *= 2;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBorder::CreateQuads(void)
{
	CQuad2D*		pcQuad;
	int				j;
	int				i;
	int				iPow2;
	SUVCornersFull	asUVs[8];
	CQuad2D*		mapcQuads[BORDER_QUADS];
	int				afEdgeLength[4];
	int				afEdgeStart[4];

	CalculateUVCoords(asUVs);

	iPow2 = 1;
	for (i = 0; i < BORDER_QUADS; i++)
	{
		if (mpcBorderParameters->meBorderStyle & iPow2)
		{
			pcQuad = mpcViewport->mcMap2D.CreateQuad(mpcBorderParameters->GetGraphicsMaterial(), mpcBorderParameters->GetGraphicsState(), mpcGraphicsViewport, 0, 0, (int)msPosition.x, (int)msPosition.y);
			mapcQuads[i] = pcQuad;

			for (j = 0; j < mpcBorderParameters->GetGraphicsMaterial()->GetNumTextures(); j++)
			{
				if (j == 0)
				{
					pcQuad->SetUVCoords(j, &asUVs[i]);
				}
				else
				{
					pcQuad->SetUVCoords(j, 0.0f, 0.0f, 1.0f, 1.0f);
				}
			}
		}
		iPow2 *= 2;
	}

	CalculateEdgePositions(afEdgeLength, afEdgeStart);
	SetQuadDimensions(mapcQuads, afEdgeLength, afEdgeStart);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBorder::CalculateUVCoords(SUVCornersFull* apsUVs)
{
	SFloat2		sTopLeft;
	SFloat2		sBottomRight;
	CRectangle	sSubImage;
	int			iTextureWidth;
	int			iTextureHeight;

	if (mpcBorderParameters->GetGraphicsMaterial()->GetNumTextures() > 0)
	{
		//You're not actually done yet, until you handle the tiled version.

		iTextureWidth = GetTexture(0)->GetWidth();
		iTextureHeight = GetTexture(0)->GetHeight();

		//Top
		sSubImage.miTop = 0;
		sSubImage.miLeft = 0;
		sSubImage.SetWidth(iTextureWidth);
		sSubImage.SetHeight(mpcBorderParameters->maiEdgeWidths[0]);
		sTopLeft	 = sSubImage.GetUVCoordinatesTopLeft(iTextureWidth, iTextureHeight);
		sBottomRight = sSubImage.GetUVCoordinatesBottomRight(iTextureWidth, iTextureHeight);
		apsUVs[1].Set(sTopLeft.x, sTopLeft.y, sBottomRight.x, sBottomRight.y);

		//Right
		sSubImage.miTop += sSubImage.GetHeight() - 1;  //Don't even ask...
		sSubImage.SetHeight(mpcBorderParameters->maiEdgeWidths[1]);
		sTopLeft	 = sSubImage.GetUVCoordinatesTopLeft(iTextureWidth, iTextureHeight);
		sBottomRight = sSubImage.GetUVCoordinatesBottomRight(iTextureWidth, iTextureHeight);
		apsUVs[3].Set(sTopLeft.x, sTopLeft.y, sBottomRight.x, sBottomRight.y);
		apsUVs[3].Rotate(1);

		//Bottom
		sSubImage.miTop += sSubImage.GetHeight();
		sSubImage.SetHeight(mpcBorderParameters->maiEdgeWidths[2]);
		sTopLeft	 = sSubImage.GetUVCoordinatesTopLeft(iTextureWidth, iTextureHeight);
		sBottomRight = sSubImage.GetUVCoordinatesBottomRight(iTextureWidth, iTextureHeight);
		apsUVs[5].Set(sTopLeft.x, sTopLeft.y, sBottomRight.x, sBottomRight.y);
		apsUVs[5].Rotate(2);

		//Left
		sSubImage.miTop += sSubImage.GetHeight() + 1;
		sSubImage.SetHeight(mpcBorderParameters->maiEdgeWidths[3]);
		sTopLeft	 = sSubImage.GetUVCoordinatesTopLeft(iTextureWidth, iTextureHeight);
		sBottomRight = sSubImage.GetUVCoordinatesBottomRight(iTextureWidth, iTextureHeight);
		apsUVs[7].Set(sTopLeft.x, sTopLeft.y, sBottomRight.x, sBottomRight.y);
		apsUVs[7].Rotate(3);

		//Top Left
		sSubImage.miTop += sSubImage.GetHeight();
		sSubImage.SetHeight(mpcBorderParameters->masCornerSizes[0].y);
		sSubImage.SetWidth(mpcBorderParameters->masCornerSizes[0].x);
		sTopLeft	 = sSubImage.GetUVCoordinatesTopLeft(iTextureWidth, iTextureHeight);
		sBottomRight = sSubImage.GetUVCoordinatesBottomRight(iTextureWidth, iTextureHeight);
		apsUVs[0].Set(sTopLeft.x, sTopLeft.y, sBottomRight.x, sBottomRight.y);

		//Top Right
		sSubImage.miTop += sSubImage.GetHeight();
		sSubImage.SetHeight(mpcBorderParameters->masCornerSizes[1].y);
		sSubImage.SetWidth(mpcBorderParameters->masCornerSizes[1].x);
		sTopLeft	 = sSubImage.GetUVCoordinatesTopLeft(iTextureWidth, iTextureHeight);
		sBottomRight = sSubImage.GetUVCoordinatesBottomRight(iTextureWidth, iTextureHeight);
		apsUVs[2].Set(sTopLeft.x, sTopLeft.y, sBottomRight.x, sBottomRight.y);

		//Bottom Right
		sSubImage.miTop += sSubImage.GetHeight();
		sSubImage.SetHeight(mpcBorderParameters->masCornerSizes[2].y);
		sSubImage.SetWidth(mpcBorderParameters->masCornerSizes[2].x);
		sTopLeft	 = sSubImage.GetUVCoordinatesTopLeft(iTextureWidth, iTextureHeight);
		sBottomRight = sSubImage.GetUVCoordinatesBottomRight(iTextureWidth, iTextureHeight);
		apsUVs[4].Set(sTopLeft.x, sTopLeft.y, sBottomRight.x, sBottomRight.y);

		//Bottom Left
		sSubImage.miTop += sSubImage.GetHeight();
		sSubImage.SetHeight(mpcBorderParameters->masCornerSizes[3].y);
		sSubImage.SetWidth(mpcBorderParameters->masCornerSizes[3].x);
		sTopLeft	 = sSubImage.GetUVCoordinatesTopLeft(iTextureWidth, iTextureHeight);
		sBottomRight = sSubImage.GetUVCoordinatesBottomRight(iTextureWidth, iTextureHeight);
		apsUVs[6].Set(sTopLeft.x, sTopLeft.y, sBottomRight.x, sBottomRight.y);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBorder::GetUsableArea(SInt2* psPosition, SInt2* psSize, SInt2* psContainerSize)
{
	psPosition->x = mpcBorderParameters->maiEdgeWidths[3];
	psPosition->y = mpcBorderParameters->maiEdgeWidths[0];
	*psSize = *psContainerSize;
	psSize->x -= (mpcBorderParameters->maiEdgeWidths[3] + mpcBorderParameters->maiEdgeWidths[1]);
	psSize->y -= (mpcBorderParameters->maiEdgeWidths[0] + mpcBorderParameters->maiEdgeWidths[2]);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBorder::SetAlpha(float fAlpha)
{
	mfAlpha = fAlpha;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBorder::SetRequiredSize(void)
{
	msRequiredSize.Init((mpcBorderParameters->maiEdgeWidths[3] + mpcBorderParameters->maiEdgeWidths[1]), 
						(mpcBorderParameters->maiEdgeWidths[0] + mpcBorderParameters->maiEdgeWidths[2]));
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsTexture* CBorder::GetTexture(int iLayer)
{
	return mpcBorderParameters->mpcGraphicsMaterial->GetTexture(iLayer);
}

