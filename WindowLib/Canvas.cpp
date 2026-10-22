#include "StandardLib/ClassDefines.h"
#include "NativeWindowFactory.h"
#include "Window.h"
#include "NativeCanvas.h"
#include "Canvas.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::Init(Ptr<CWindow> pWindow, Ptr<CCanvasDraw> pDraw, EColourFormat eFormat, EColourOrder eOrder, ERGBColourBits eColourBits, ERGBAlphaBits eAlphaBits)
{
	PreInit();

	meColourFormat = eFormat;
	meColourOrder = eOrder;
	meColourBits = eColourBits;
	meAlphaBits = eAlphaBits;

	mpcNativeCanvas = NULL;
	CComponent::Init(pWindow);

	mpCanvasDraw = pDraw;
	mbTransparent = false;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::Init(Ptr<CWindow> pWindow, Ptr<CCanvasDraw> pDraw)
{
	PreInit();

	meColourFormat = pWindow->GetColourFormat();
	meColourOrder = pWindow->GetColourOrder();
	meColourBits = pWindow->GetColourBits();
	meAlphaBits = pWindow->GetAlphaBits();

	mpcNativeCanvas = NULL;
	CComponent::Init(pWindow);

	mpCanvasDraw = pDraw;
	mbTransparent = false;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::Free(void)
{
	DestroyNativeCanvas();

	meColourFormat = CFT_Unknown;
	meColourOrder = CCO_Unknown;
	meColourBits = CRGB_Unknown;
	meAlphaBits = ARGB_Unknown;

	CComponent::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::Class(void)
{
	CComponent::Class();

	U_Pointer(mpcNativeCanvas);
	U_Enum(meColourFormat);
	U_Enum(meColourOrder);
	U_Enum(meColourBits);
	U_Enum(meAlphaBits);
	M_Pointer(mpCanvasDraw);
	M_Pointer(mpContainer);
	U_Bool(mbTransparent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCanvas::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCanvas::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8* CCanvas::GetPixelData(void)
{
	return mpcNativeCanvas->GetPixelData();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCanvas::Draw(void)
{
	Ptr<CCanvas>			pWindowCanvas;
	CInt32Rect				cClip;
	bool					bClipped;

	if (HasNativeChanged())
	{
		DestroyNativeCanvas();
		CreateNativeCanvas();
	}

	//A transparent canvas starts each draw empty so that only what is drawn this time shows over what is behind it.
	if (mbTransparent)
	{
		mpcNativeCanvas->Clear();
	}

	//The canvas draws itself and is copied onto the window before its children so that they are drawn over it.
	if (mpCanvasDraw)
	{
		mpCanvasDraw->Draw(this);
	}

	//A canvas inside a component that clips its children, like a scroll pane, is only copied where it shows.
	pWindowCanvas = mpWindow->GetCanvas();
	if (&pWindowCanvas != this)
	{
		bClipped = GetClip(&cClip);
		if (!bClipped)
		{
			pWindowCanvas->DrawCanvas(mcActualBounds.GetLeft(), mcActualBounds.GetTop(), this);
		}
		else if ((cClip.GetWidth() > 0) && (cClip.GetHeight() > 0))
		{
			pWindowCanvas->DrawCanvas(mcActualBounds.GetLeft(), mcActualBounds.GetTop(), this, &cClip);
		}
	}

	CComponent::Draw();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::CreateNativeCanvas(void)
{
	CNativeWindowFactory* pcFactory;

	pcFactory = mpWindow->GetFactory();
	mpcNativeCanvas = pcFactory->CreateNativeCanvas(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::DestroyNativeCanvas(void)
{
	CNativeWindowFactory* pcFactory;

	if (mpcNativeCanvas)
	{
		pcFactory = mpWindow->GetFactory();
		pcFactory->DestroyNativeCanvas(mpcNativeCanvas);
		mpcNativeCanvas = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCanvas::HasNativeChanged(void)
{
	SInt32Vec2	sSize;

	if (mpcNativeCanvas == NULL)
	{
		return true;
	}

	sSize = mpcNativeCanvas->GetSize();
	if ((mcActualBounds.GetWidth() != sSize.x) ||
		(mcActualBounds.GetHeight() != sSize.y))
	{
		return true;
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::CopyCanvas(Ptr<CCanvas> pSourceCanvas)
{
	CNativeCanvas*	pcSourceNativeCanvas;
	CNativeCanvas*	pcDestNativeCanvas;
	Ptr<CContainer>	pContainer;

	pcDestNativeCanvas = GetNativeCanvas();
	pcSourceNativeCanvas = pSourceCanvas->GetNativeCanvas();
	pContainer = pSourceCanvas->GetContainer();

	if (pcDestNativeCanvas)
	{
		pcDestNativeCanvas->CopyCanvas(pcSourceNativeCanvas);
	}

	SetContainer(pContainer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CCanvas::GetImageOrNull(void)
{
	return mpcNativeCanvas->GetImageOrNull();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::DrawCanvas(int iX, int iY, Ptr<CCanvas> pSourceCanvas, CInt32Rect* pcClip)
{
	CNativeCanvas*	pcSourceNativeCanvas;

	pcSourceNativeCanvas = pSourceCanvas->GetNativeCanvas();
	mpcNativeCanvas->DrawCanvas(iX, iY, pcSourceNativeCanvas, pcClip);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::DrawBox(CInt32Rect* pcRect, bool bFilled, ARGB32 sColour)
{
	mpcNativeCanvas->DrawBox(pcRect, bFilled, sColour);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::DrawPixel(int iX, int iY, ARGB32 sColour)
{
	mpcNativeCanvas->DrawPixel(iX, iY, sColour);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::DrawImage(int iX, int iY, Ptr<CImage> pImage)
{
	mpcNativeCanvas->DrawImage(iX, iY, pImage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCanvas::IsValid(void)
{
	if (meColourFormat == CFT_Unknown)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCanvas::SetContainer(Ptr<CContainer> pContainer)
{
	if (!mpContainer)
	{
		if (pContainer)
		{
			AddComponent(pContainer);
		}
		mpContainer = pContainer;
		return true;
	}
	else
	{
		return gcLogger.Error2(__METHOD__, " Container is already set on Canvas.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCanvas::ClearContainer(void)
{
	bool	bResult;

	if (mpContainer)
	{
		bResult = RemoveComponent(mpContainer);
		if (!bResult)
		{
			return gcLogger.Error2(__METHOD__, " Container is set on Canvas but is not a child component.", NULL);
		}
		mpContainer = NULL;
	}
	return true;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CCanvas::GetDesiredSize(void)
{
	//A desired size of -1, -1 on the canvas means use the size its draw wants.
	if ((msDesiredSize.x == -1) && (msDesiredSize.y == -1) && mpCanvasDraw.IsNotNull())
	{
		return mpCanvasDraw->GetDesiredSize();
	}
	return msDesiredSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
// A canvas is opaque unless it is made transparent; blending it onto what
// is behind is slower than copying it.  A transparent canvas has an 8 bit
// opacity channel and its colours are multiplied by its opacity, so only
// canvases with 8 bit colour channels can be transparent.
//
//////////////////////////////////////////////////////////////////////////
bool CCanvas::SetTransparent(bool bTransparent)
{
	if (bTransparent && (meColourBits != CRGB_24bit))
	{
		return gcLogger.Error2(__METHOD__, " Only a canvas with 8 bit colour channels can be transparent.", NULL);
	}

	if (mbTransparent != bTransparent)
	{
		//The native canvas has a different format so it is made again when next drawn.
		mbTransparent = bTransparent;
		DestroyNativeCanvas();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The format with an opacity channel where the canvas's format has one or
// an ignored channel.
//
//////////////////////////////////////////////////////////////////////////
EColourFormat CCanvas::GetTransparentColourFormat(void)
{
	if ((meColourFormat == CFT_XRGB) || (meColourFormat == CFT_ARGB))
	{
		return CFT_ARGB;
	}
	return CFT_RGBA;
}


//////////////////////////////////////////////////////////////////////////
//
// The format of the canvas's image, with an opacity channel if the canvas
// is transparent.
//
//////////////////////////////////////////////////////////////////////////
EColourFormat CCanvas::GetColourFormat(void)
{
	if (mbTransparent)
	{
		return GetTransparentColourFormat();
	}
	return meColourFormat;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ERGBAlphaBits CCanvas::GetAlphaBits(void)
{
	if (mbTransparent)
	{
		return ARGB_8bit;
	}
	return meAlphaBits;
}


EColourOrder CCanvas::GetColourOrder(void) { return meColourOrder; }
ERGBColourBits CCanvas::GetColourBits(void) { return meColourBits; }
bool CCanvas::IsTransparent(void) { return mbTransparent; }
CNativeCanvas* CCanvas::GetNativeCanvas(void) { return mpcNativeCanvas; }
Ptr<CCanvasDraw> CCanvas::GetCanvasDraw(void) { return mpCanvasDraw; }
void CCanvas::SetCanvasDraw(Ptr<CCanvasDraw> pDraw) { mpCanvasDraw = pDraw; }
Ptr<CContainer> CCanvas::GetContainer(void) { return mpContainer; }

