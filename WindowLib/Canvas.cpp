#include "NativeWindowFactory.h"
#include "NativeCanvas.h"
#include "Canvas.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::Init(CNativeWindowFactory* pcWindowFactory)
{
	Init(CF_Unknown, -1, -1, pcWindowFactory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::Init(EColourFormat eFormat, int32 iWidth, int32 iHeight, CNativeWindowFactory* pcWindowFactory)
{
	mpcNativeCanvas = pcWindowFactory->CreateNativeCanvas(this);
	meFormat = eFormat;
	miWidth = iWidth;
	miHeight = iHeight;

	CBasicComponent::Init(mpcNativeCanvas);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvas::Kill(void)
{
	mpcNativeCanvas->mpcWindowFactory->DestroyNativeCanvas(mpcNativeCanvas);
	mpcNativeCanvas = NULL;

	meFormat = CF_Unknown;
	miWidth = -1;
	miHeight = -1;

	CBasicComponent::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EColourFormat CCanvas::GetFormat(void) { return meFormat; }
int32 CCanvas::GetWidth(void) { return miWidth; }
int32 CCanvas::GetHeight(void) { return miHeight; }

