#ifndef __IMAGE_DRAW_H__
#define __IMAGE_DRAW_H__
#include "StandardLib/Pointer.h"
#include "Image.h"
#include "ImageAccessor.h"
#include "ImageColour.h"
#include "ImageBlitter.h"
#include "ImageRowBlitterFactory.h"


class CImageDraw
{
protected:
	CImage*					mpcImage;
	CImageAccessor*			mpcAccessor;
	SImageColour			msColour;

public:
	void	Init(CImage* ppImage);  //A (*) pointer instead of Ptr pointer because CImageDraw is not an object.  Note that the Image should be added into FreeObjects to ensure it's not evicted whilst this pointer is in use.
	void	Kill(void);

	void	SetColour(CImageColour* pcColour);

	void	DrawPixel(int x, int y);

	void	DrawBox(CRectangle* pcRetangle, bool bFilled);
	void	DrawBox(int iLeft, int iTop, int iRight, int iBottom, bool bFilled);
	void	DrawBox(bool bFilled);

	void	DrawLine(int x0, int y0, int x1, int y1);
	void	DrawHorizontalLine(int x0, int x1, int y);
	void	DrawVerticalLine(int x, int y0, int y1);

	bool	DrawCel(int x, int y, Ptr<CImageCel> pCel, CRectangle* pcClip = NULL);
	bool	DrawCel(int x, int y, int iWidth, int iHeight, Ptr<CImageCel> pCel, CRectangle* pcClip = NULL);
};


#endif // __IMAGE_DRAW_H__

