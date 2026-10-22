/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include "StandardLib/Objects.h"
#include "SupportLib/ImageCopier.h"
#include "SupportLib/ImagePremultipliedBlend.h"
#include "WindowLib/Canvas.h"
#include "X11WindowFactory.h"
#include "X11Canvas.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Canvas::Init(CCanvas* pcCanvas, CNativeWindowFactory* pcWindowFactory)
{
    CNativeCanvas::Init(pcCanvas, pcWindowFactory);
    mpuiPixelData = NULL;
    mpImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Canvas::Kill(void)
{
    //The image references the pixel memory so it must be released before the memory is freed.
    mpImage = NULL;
    if (mpuiPixelData)
    {
        free(mpuiPixelData);
        mpuiPixelData = NULL;
    }
    CNativeCanvas::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CX11Canvas::CreateNativeCanvas(void)
{
    //This is split from .Init() so that it can fail on its own.

    SInt32Vec2  sSize;

    sSize = mpcCanvas->GetActualSize();
    if ((sSize.x <= 0) || (sSize.y <= 0))
    {
        SetSize(-1, -1);
        return false;
    }

    //Like the GDI canvas' DIB section the pixels start all zero; black, and transparent if the canvas is.
    mpuiPixelData = (uint8*)calloc((size_t)sSize.x * (size_t)sSize.y, 4);
    if (mpuiPixelData == NULL)
    {
        SetSize(-1, -1);
        return false;
    }

    //A transparent canvas uses X for its opacity and, like the reference canvas, holds its colours multiplied by it.
    if (mpcCanvas->IsTransparent())
    {
        mpImage = OMalloc<CImage>(sSize.x, sSize.y, (void*)mpuiPixelData, PT_uint8, IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, IMAGE_OPACITY, CHANNEL_STOP);
    }
    else
    {
        mpImage = OMalloc<CImage>(sSize.x, sSize.y, (void*)mpuiPixelData, PT_uint8, IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, IMAGE_IGNORED, CHANNEL_STOP);
    }
    SetSize(sSize.x, sSize.y);
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8* CX11Canvas::GetPixelData(void)
{
    return mpuiPixelData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Canvas::CopyCanvas(CNativeCanvas* pcSourceCanvas)
{
    CX11Canvas*     pcX11Source;
    SInt32Vec2      sSize;
    int             y;

    if (!pcSourceCanvas)
    {
        return;
    }

    pcX11Source = (CX11Canvas*)pcSourceCanvas;
    if ((pcX11Source->mpuiPixelData == NULL) || (mpuiPixelData == NULL))
    {
        return;
    }

    sSize.x = (msSize.x < pcX11Source->msSize.x) ? msSize.x : pcX11Source->msSize.x;
    sSize.y = (msSize.y < pcX11Source->msSize.y) ? msSize.y : pcX11Source->msSize.y;

    for (y = 0; y < sSize.y; y++)
    {
        memcpy(&((uint32*)mpuiPixelData)[y * msSize.x], &((uint32*)pcX11Source->mpuiPixelData)[y * pcX11Source->msSize.x], sSize.x * sizeof(uint32));
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CX11Canvas::GetImageOrNull(void)
{
    return mpImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Canvas::DrawBox(CInt32Rect* pcRectangle, bool bFilled, ARGB32 sColour)
{
    int     iLeft;
    int     iTop;
    int     iRight;
    int     iBottom;

    //The rectangle includes its right and bottom edges but Fill does not.
    iLeft = pcRectangle->msTopLeft.x;
    iTop = pcRectangle->msTopLeft.y;
    iRight = pcRectangle->msBottomRight.x + 1;
    iBottom = pcRectangle->msBottomRight.y + 1;

    if (bFilled)
    {
        Fill(iLeft, iTop, iRight, iBottom, sColour);
    }
    else
    {
        Fill(iLeft, iTop, iRight, iTop + 1, sColour);
        Fill(iLeft, iBottom - 1, iRight, iBottom, sColour);
        Fill(iLeft, iTop + 1, iLeft + 1, iBottom - 1, sColour);
        Fill(iRight - 1, iTop + 1, iRight, iBottom - 1, sColour);
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Canvas::DrawPixel(int32 iX, int32 iY, ARGB32 sColour)
{
    Fill(iX, iY, iX + 1, iY + 1, sColour);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Canvas::DrawCanvas(int iX, int iY, CNativeCanvas* pcSource, CInt32Rect* pcClip)
{
    CX11Canvas*     pcX11Source;
    CInt32Rect      cDest;
    CInt32Rect      cCanvas;
    int             iSourceX;
    int             iSourceY;
    int             y;
    int             iWidth;

    if (!pcSource || (mpuiPixelData == NULL))
    {
        return;
    }

    pcX11Source = (CX11Canvas*)pcSource;
    if (pcX11Source->mpuiPixelData == NULL)
    {
        return;
    }

    //Only the part of the source inside the clip is copied.
    cDest.Init(iX, iY, iX + pcX11Source->msSize.x, iY + pcX11Source->msSize.y);
    if (pcClip)
    {
        cDest.ClipTo(pcClip);
    }
    if ((cDest.GetWidth() <= 0) || (cDest.GetHeight() <= 0))
    {
        return;
    }
    if (pcX11Source->mpcCanvas->IsTransparent())
    {
        //What the transparent canvas didn't cover must show through so it is blended rather than copied.
        BlendPremultipliedImage(pcX11Source->mpImage, mpImage, iX, iY, &cDest);
        return;
    }

    //BitBlt clips to the destination, the copy must too.
    cCanvas.Init(0, 0, msSize.x, msSize.y);
    cDest.ClipTo(&cCanvas);
    if ((cDest.GetWidth() <= 0) || (cDest.GetHeight() <= 0))
    {
        return;
    }

    iSourceX = cDest.GetLeft() - iX;
    iSourceY = cDest.GetTop() - iY;
    iWidth = cDest.GetWidth();

    for (y = 0; y < cDest.GetHeight(); y++)
    {
        memcpy(&((uint32*)mpuiPixelData)[(cDest.GetTop() + y) * msSize.x + cDest.GetLeft()],
               &((uint32*)pcX11Source->mpuiPixelData)[(iSourceY + y) * pcX11Source->msSize.x + iSourceX],
               iWidth * sizeof(uint32));
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Canvas::DrawCel(int iX, int iY, Ptr<CImageCel> pSource)
{
    if (pSource.IsNull() || mpImage.IsNull())
    {
        return;
    }

    CImageCopier::Copy(pSource, mpImage, iX, iY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Canvas::DrawImage(int iX, int iY, Ptr<CImage> pSource)
{
    if (pSource.IsNull() || mpImage.IsNull())
    {
        return;
    }

    CImageCopier::Copy(pSource, mpImage, iX, iY);
}


//////////////////////////////////////////////////////////////////////////
//
// Fills left to right - 1 and top to bottom - 1, clipped to the canvas.
// A transparent canvas gets the colour multiplied by its opacity, an
// opaque one gets the colour with X zero, as GDI writes it.
//
//////////////////////////////////////////////////////////////////////////
void CX11Canvas::Fill(int iLeft, int iTop, int iRight, int iBottom, ARGB32 sColour)
{
    uint32      uiRed;
    uint32      uiGreen;
    uint32      uiBlue;
    uint32      uiAlpha;
    uint32      uiPixel;
    uint32*     puiRow;
    int         x;
    int         y;

    if (mpuiPixelData == NULL)
    {
        return;
    }

    if (iLeft < 0)
    {
        iLeft = 0;
    }
    if (iTop < 0)
    {
        iTop = 0;
    }
    if (iRight > msSize.x)
    {
        iRight = msSize.x;
    }
    if (iBottom > msSize.y)
    {
        iBottom = msSize.y;
    }

    Get32BitColour(&uiRed, &uiGreen, &uiBlue, &uiAlpha, sColour);
    if (mpcCanvas->IsTransparent())
    {
        uiRed = (uiRed * uiAlpha + 127) / 255;
        uiGreen = (uiGreen * uiAlpha + 127) / 255;
        uiBlue = (uiBlue * uiAlpha + 127) / 255;
        uiPixel = (uiAlpha << 24) | (uiRed << 16) | (uiGreen << 8) | uiBlue;
    }
    else
    {
        uiPixel = (uiRed << 16) | (uiGreen << 8) | uiBlue;
    }

    for (y = iTop; y < iBottom; y++)
    {
        puiRow = &((uint32*)mpuiPixelData)[y * msSize.x];
        for (x = iLeft; x < iRight; x++)
        {
            puiRow[x] = uiPixel;
        }
    }
}

