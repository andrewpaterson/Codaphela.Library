#ifndef __IMAGE_DRAW_H__
#define __IMAGE_DRAW_H__
#include "StandardLib/Pointer.h"
#include "Image.h"
#include "ImageAccessor.h"
#include "ImageColour.h"


class CImageDraw
{
protected:
	CImage*				mpcImage;
	CImageAccessor*		mpcAccessor;
	SImageColour		msColour;

public:
	void	Init(CImage* ppImage);  //* pointer instead of Ptr pointer because CImageDraw is not an object.
	void	Kill(void);

	void	SetColour(CImageColour* pcColour);

	void	DrawPixel(int x, int y);

	void	DrawBox(CRectangle* pcRetangle, bool bFilled);
	void	DrawBox(int iLeft, int iTop, int iRight, int iBottom, bool bFilled);
	void	DrawBox(bool bFilled);

	void	DrawLine(int x0, int y0, int x1, int y1);
	void	DrawHorizontalLine(int x0, int x1, int y);
	void	DrawVerticalLine(int x, int y0, int y1);

	void	DrawImage(int x, int y, CImage* pcSource);
};


#endif // __IMAGE_DRAW_H__

