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
    mpDestImage = NULL;
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
    M_Pointer(mpDestImage);
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
    SInt32Vec2      sSize;
    bool            bResult;
    Ptr<CImage>     pImage;
    
    if (pCanvas->IsValid())
    {
        sSize = pCanvas->GetActualSize();
        pImage = pCanvas->GetImageOrNull();
        if (pImage.IsNull())
        {
            if (mpDestImage.IsNull())
            {
                mpDestImage = OMalloc<CImage>(sSize.x, sSize.y, pCanvas->GetColourFormat(), pCanvas->GetColourOrder(), pCanvas->GetColourBits(), pCanvas->GetAlphaBits());
                pImage = mpDestImage;
            }
        }
        else
        {
            if (mpDestImage.IsNotNull())
            {
                mpDestImage = NULL;
            }
        }

        if (mpBlitterCache.IsNull())
        {
            mpBlitterCache = OMalloc<CImageCelBlitterCache>(pImage);
        }
        else if (!mpBlitterCache->Matches(pImage))
        {
            mpBlitterCache->Clear(pImage);
        }
        
        mpMaps->SetCacheAndViewport(mpBlitterCache, pImage);

        if (!mpMaps->HasCelBlitters())
        {
            bResult = mpMaps->CreateCelBlitters();
        }
        else
        {
            bResult = true;
        }
        if (bResult)
        {
            bResult = mpMaps->Blit(false);
        }

        if (mpDestImage.IsNotNull())
        {
            pCanvas->DrawCanvas(0, 0, mpDestImage);
        }
    }
    return true;
}

