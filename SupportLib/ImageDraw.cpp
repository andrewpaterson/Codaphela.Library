#include "ImageAccessorCreator.h"
#include "ImageDraw.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDraw::Init(Ptr<CImage> pImage)
{
	CImageColourRGB	cColour;

	mpImage = pImage;
	mpcAccessor = NULL;
	
	cColour.Init(1.0f, 1.0f, 1.0f);
	SetColour(&cColour);
	cColour.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDraw::Kill(void)
{
	if (mpcAccessor)
	{
		mpcAccessor->Kill();
		mpcAccessor = NULL;
	}
	mpImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDraw::SetColour(CImageColour* pcColour)
{
	if (mpcAccessor)
	{
		mpcAccessor->Kill();
		mpcAccessor = NULL;
	}

	mpcAccessor = CImageAccessorCreator::Create(mpImage, pcColour);
	mpcAccessor->MakeColour(&msColour, pcColour);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDraw::DrawBox(CRectangle* pcRetangle, bool bFilled)
{
	int					x;
	int					y;

	if (bFilled)
	{
		for (y = pcRetangle->miTop; y < pcRetangle->miBottom; y++)
		{
			for (x = pcRetangle->miLeft; x < pcRetangle->miRight; x++)
			{
				mpcAccessor->Set(x, y, &msColour);
			}
		}
	}
	else
	{
		for (x = pcRetangle->miLeft; x < pcRetangle->miRight; x++)
		{
			mpcAccessor->Set(x, pcRetangle->miTop, &msColour);
			mpcAccessor->Set(x, pcRetangle->miBottom - 1, &msColour);
		}

		for (y = pcRetangle->miTop; y < pcRetangle->miBottom; y++)
		{
			mpcAccessor->Set(pcRetangle->miLeft, y, &msColour);
			mpcAccessor->Set(pcRetangle->miRight - 1, y, &msColour);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDraw::DrawBox(bool bFilled)
{
	CRectangle			cRectangle;

	cRectangle.Init(0, 0, mpImage->GetWidth(), mpImage->GetHeight());
	DrawBox(&cRectangle, bFilled);
}

