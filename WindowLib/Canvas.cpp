#include "StandardLib/ClassDefines.h"
#include "NativeWindowFactory.h"
#include "NativeCanvas.h"
#include "Canvas.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::Init(Ptr<CWindow> pWindow, CNativeWindowFactory* pcFactory, Ptr<CCanvasDraw> pDraw)
{
	PreInit();

	Init(pWindow, CF_Unknown, -1, -1, pDraw, pcFactory);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::Init(Ptr<CWindow> pWindow, EColourFormat eFormat, int32 iWidth, int32 iHeight, Ptr<CCanvasDraw> pDraw, CNativeWindowFactory* pcFactory)
{
	meFormat = eFormat;
	miWidth = iWidth;
	miHeight = iHeight;

	mpcNativeCanvas = pcFactory->CreateNativeCanvas(this);
	CComplexComponent::Init(pWindow, mpcNativeCanvas);

	mpCanvasDraw = pDraw;
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
	miWidth = -1;
	miHeight = -1;

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
	U_Int32(miWidth);
	U_Int32(miHeight);
	M_Pointer(mpCanvasDraw);
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
	CComplexComponent::Draw();
	if (mpCanvasDraw)
	{
		mpCanvasDraw->Draw(this);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::LayoutActualSize(void)
{
	if (IsValid())
	{
		CComplexComponent::SetActualSize(miWidth, miHeight);
		Layout(msPosition, msActualSize);
		//mcFocus.Update(mcInput.GetPointer()->GetX(), mcInput.GetPointer()->GetY());
	}
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
void CCanvas::DrawBox(CRectangle* pcRect, bool bFilled, ARGB32 sColour)
{
	mpcNativeCanvas->DrawBox(pcRect, bFilled, sColour);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::DrawPixel(int32 iX, int32 iY, ARGB32 sColour)
{
	mpcNativeCanvas->DrawPixel(iX, iY, sColour);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCanvas::IsValid(void)
{
	if ((meFormat == CF_Unknown) ||
		(miWidth == -1) ||
		(miHeight == -1))
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
int32 CCanvas::GetWidth(void) { return miWidth; }
int32 CCanvas::GetHeight(void) { return miHeight; }
CNativeCanvas* CCanvas::GetNativeCanvas(void) { return mpcNativeCanvas; }
Ptr<CCanvasDraw> CCanvas::GetCanvasDraw(void) { return mpCanvasDraw; }
Ptr<CContainer> CCanvas::GetContainer(void) { return mpContainer; }

