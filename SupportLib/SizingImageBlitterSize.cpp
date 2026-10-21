#include "SubImage.h"
#include "SizingImageBlitterSize.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSizingImageBlitterSize::Init(void)
{
	msSizedTo.Init(-1, -1);
	maiSizedSourceX.Init();
	maiSizedSourceY.Init();
	maiSizedDestX.Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSizingImageBlitterSize::Kill(void)
{
	maiSizedDestX.Kill();
	maiSizedSourceY.Kill();
	maiSizedSourceX.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CSizingImageBlitterSize::Matches(int32 iDestWidth, int32 iDestHeight)
{
	if ((msSizedTo.x == iDestWidth) && (msSizedTo.y == iDestHeight))
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSizingImageBlitterSize::CacheBresenhamIndices(CArrayInt32* paiSamples, int32 iDestLength, int32 iFullLength, int32 iOffset)
{
	int32*	paiSample;
	int32	i;
	int32	iSample;
	int32	iError;

	//Destination position i is drawn from the cel position under its centre, (2i + 1) * iFullLength / (2 * iDestLength).
	//Bresenham's error term counts in steps of 1 / (2 * iDestLength) so no division is needed.  Stretching repeats a cel
	//position, shrinking skips cel positions.
	paiSamples->Resize(iDestLength);
	paiSample = paiSamples->GetData();

	iSample = 0;
	iError = iFullLength;
	for (i = 0; i < iDestLength; i++)
	{
		while (iError >= 2 * iDestLength)
		{
			iSample++;
			iError -= 2 * iDestLength;
		}
		paiSample[i] = iSample + iOffset;
		iError += 2 * iFullLength;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CSizingImageBlitterSize::PopulateSizingTables(CSubImage* pcSubImage, int32 iDestWidth, int32 iDestHeight)
{
	int32		iFullWidth;
	int32		iFullHeight;
	int32		iSourceLeft;
	int32*		paiSourceX;
	int32*		paiDestX;
	int32		x;
	int32		xDest;

	if (Matches(iDestWidth, iDestHeight))
	{
		return false;
	}

	iFullWidth = pcSubImage->GetFullWidth();
	iFullHeight = pcSubImage->GetFullHeight();

	//Column x of the full cel is column iSourceLeft + x of the source image, outside the image in the transparent borders.
	iSourceLeft = pcSubImage->mcImageRect.msTopLeft.x - pcSubImage->msOffsetTopLeft.x;
	CacheBresenhamIndices(&maiSizedSourceX, iDestWidth, iFullWidth, iSourceLeft);
	CacheBresenhamIndices(&maiSizedSourceY, iDestHeight, iFullHeight, -pcSubImage->msOffsetTopLeft.y);

	//A segment from x to xEnd of the full cel is drawn from maiSizedDestX[x] to maiSizedDestX[xEnd].  When shrinking
	//both can be the same column and the segment is not drawn at all.
	maiSizedDestX.Resize(iFullWidth + 1);
	paiDestX = maiSizedDestX.GetData();
	paiSourceX = maiSizedSourceX.GetData();
	xDest = 0;
	for (x = 0; x <= iFullWidth; x++)
	{
		while ((xDest < iDestWidth) && (paiSourceX[xDest] - iSourceLeft < x))
		{
			xDest++;
		}
		paiDestX[x] = xDest;
	}

	msSizedTo.Init(iDestWidth, iDestHeight);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int32* CSizingImageBlitterSize::GetSizedSourceX(void)
{
	return maiSizedSourceX.GetData();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int32* CSizingImageBlitterSize::GetSizedSourceY(void)
{
	return maiSizedSourceY.GetData();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int32* CSizingImageBlitterSize::GetSizedDestX(void)
{
	return maiSizedDestX.GetData();
}

