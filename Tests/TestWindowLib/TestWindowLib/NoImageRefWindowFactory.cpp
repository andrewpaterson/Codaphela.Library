#include "NoImageRefWindowFactory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CNoImageRefCanvas::GetImageOrNull(void)
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNativeCanvas* CNoImageRefWindowFactory::CreateNativeCanvas(CCanvas* pcCanvas)
{
	CNoImageRefCanvas*	pcNativeCanvas;
	bool				bResult;

	pcNativeCanvas = NativeMalloc<CNoImageRefCanvas>();
	pcNativeCanvas->Init(pcCanvas, this);
	bResult = pcNativeCanvas->CreateNativeCanvas();
	if (!bResult)
	{
		DestroyNativeCanvas(pcNativeCanvas);
		return NULL;
	}
	return pcNativeCanvas;
}

