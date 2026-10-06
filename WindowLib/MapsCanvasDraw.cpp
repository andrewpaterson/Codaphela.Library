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
    mpBlitterCache = NULL;
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
    M_Pointer(mpBlitterCache);
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
  
    SInt32Vec2                  sSize;
    Ptr<CImage>                 pDestImage;
    bool                        bResult;
    bool                        bExternalImage;
    
    if (pCanvas->IsValid())
    {
        sSize = pCanvas->GetActualSize();
        pDestImage = pCanvas->GetImageOrNull();
        if (pDestImage.IsNull())
        {
            bExternalImage = true;
            pDestImage = OMalloc<CImage>(sSize.x, sSize.y, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
        }
        else
        {
            bExternalImage = false;
        }

        if (mpBlitterCache.IsNull())
        {
            mpBlitterCache = OMalloc<CImageCelBlitterCache>(pDestImage);
        }
        else if (!mpBlitterCache->Matches(pDestImage))
        {
            mpBlitterCache->Clear(pDestImage);
        }
        
        mpMaps->SetCacheAndViewport(mpBlitterCache, pDestImage);
        bResult = mpMaps->CreateCelBlitters();
        if (bResult)
        {
            bResult = mpMaps->Blit(false);
        }

        if (bExternalImage)
        {
            pCanvas->DrawCanvas(0, 0, pDestImage);
        }
    }
    return true;
}

