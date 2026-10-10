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

	if (HasNativeChanged())
	{
		DestroyNativeCanvas();
		CreateNativeCanvas();
	}

	CComponent::Draw();
	if (mpCanvasDraw)
	{
		mpCanvasDraw->Draw(this);
	}

	pWindowCanvas = mpWindow->GetCanvas();
	if (&pWindowCanvas != this)
	{
		pWindowCanvas->DrawCanvas(msPosition.x, msPosition.y, this);
	}
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
	if ((msActualSize.x != sSize.x) ||
		(msActualSize.y != sSize.y))
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
void CCanvas::DrawCanvas(int iX, int iY, Ptr<CCanvas> pSourceCanvas)
{
	CNativeCanvas*	pcSourceNativeCanvas;

	pcSourceNativeCanvas = pSourceCanvas->GetNativeCanvas();
	mpcNativeCanvas->DrawCanvas(iX, iY, pcSourceNativeCanvas);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::DrawBox(CRectangle* pcRect, bool bFilled, ARGB32 sColour)
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
void CCanvas::SetRequiredSize(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EColourFormat CCanvas::GetColourFormat(void) { return meColourFormat; }
EColourOrder CCanvas::GetColourOrder(void) { return meColourOrder; }
ERGBColourBits CCanvas::GetColourBits(void) { return meColourBits; }
ERGBAlphaBits CCanvas::GetAlphaBits(void) { return meAlphaBits;  }
CNativeCanvas* CCanvas::GetNativeCanvas(void) { return mpcNativeCanvas; }
Ptr<CCanvasDraw> CCanvas::GetCanvasDraw(void) { return mpCanvasDraw; }
Ptr<CContainer> CCanvas::GetContainer(void) { return mpContainer; }

