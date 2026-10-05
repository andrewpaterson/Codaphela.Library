#include "StandardLib/ClassDefines.h"
#include "WindowLib/Canvas.h"
#include "MapsCanvasDraw.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapsCanvasDraw::Init(Ptr<CMaps> pMaps)
{
    PreInit();
    CCanvasDraw::Init();
    mpMaps = pMaps;
    PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapsCanvasDraw::Class(void)
{
    CCanvasDraw::Class();
    M_Pointer(mpMaps);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapsCanvasDraw::Free(void)
{
    CCanvasDraw::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapsCanvasDraw::Save(CObjectWriter* pcFile)
{
    CCanvasDraw::Save(pcFile);
    return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapsCanvasDraw::Load(CObjectReader* pcFile)
{
    CCanvasDraw::Load(pcFile);
    return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapsCanvasDraw::Draw(Ptr<CCanvas> pCanvas)
{
    Ptr<CImageCelBlitterCache>	pCache;
    SInt32Vec2                  sSize;
    Ptr<CImage>                 pDestImage;
    bool                        bResult;
    
    if (pCanvas->IsValid())
    {
        sSize = pCanvas->GetActualSize();
        pDestImage = OMalloc<CImage>(sSize.x, sSize.y, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
        xxx;  //We just set the dest image here but never seem to tie it to the Image in the CWinRefCanvas.
        pCache = OMalloc<CImageCelBlitterCache>(pDestImage);
        mpMaps->SetCacheAndViewport(pCache, pDestImage);
        bResult = mpMaps->CreateCelBlitters();
        if (bResult)
        {
            bResult = mpMaps->Blit(false);
        }
    }
    return true;
}

