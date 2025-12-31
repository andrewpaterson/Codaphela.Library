#include "StandardLib/ClassDefines.h"
#include "NativeWindowFactory.h"
#include "Window.h"
#include "NativeCanvas.h"
#include "Canvas.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CCanvas> CCanvas::Init(Ptr<CWindow> pWindow, EColourFormat eFormat, Ptr<CCanvasDraw> pDraw)
{
	PreInit();

	meFormat = eFormat;

	mpcNativeCanvas = NULL;
	CComplexComponent::Init(pWindow);

	mpCanvasDraw = pDraw;

	PostInit();
	
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::Free(void)
{
	if (mpcNativeCanvas)
	{
		mpcNativeCanvas->mpcWindowFactory->DestroyNativeCanvas(mpcNativeCanvas);
		mpcNativeCanvas = NULL;
	}

	meFormat = CF_Unknown;

	CComplexComponent::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::Class(void)
{
	CComplexComponent::Class();

	U_Pointer(mpcNativeCanvas);
	U_Enum(meFormat);
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
	CNativeWindowFactory*	pcFactory;
	Ptr<CCanvas>			pWindowCanvas;

	if (HasNativeChanged())
	{
		pcFactory = mpWindow->GetFactory();
		if (mpcNativeCanvas)
		{
			pcFactory->DestroyNativeCanvas(mpcNativeCanvas);
			mpcNativeCanvas = NULL;
		}
		mpcNativeCanvas = pcFactory->CreateNativeCanvas(this);
	}

	CComplexComponent::Draw();
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
bool CCanvas::HasNativeChanged(void)
{
	SInt2	sSize;

	if (mpcNativeCanvas == NULL)
	{
		return true;
	}

	sSize = mpcNativeCanvas->GetSize();
	if ((msActualSize.x != sSize.x) ||
		(msActualSize.y != sSize.y))
	{
		return false;
	}
	return true;
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
bool CCanvas::IsValid(void)
{
	if (meFormat == CF_Unknown)
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
EColourFormat CCanvas::GetFormat(void) { return meFormat; }
CNativeCanvas* CCanvas::GetNativeCanvas(void) { return mpcNativeCanvas; }
Ptr<CCanvasDraw> CCanvas::GetCanvasDraw(void) { return mpCanvasDraw; }
Ptr<CContainer> CCanvas::GetContainer(void) { return mpContainer; }

