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

	void	DrawPixel(int32 x, int32 y);

	void	DrawBox(CRectangle* pcRetangle, bool bFilled);
	void	DrawBox(int32 iLeft, int32 iTop, int32 iRight, int32 iBottom, bool bFilled);
	void	DrawBox(bool bFilled);

	void	DrawLine(int32 x0, int32 y0, int32 x1, int32 y1);
	void	DrawHorizontalLine(int32 x0, int32 x1, int32 y);
	void	DrawVerticalLine(int32 x, int32 y0, int32 y1);
};


#endif // __IMAGE_DRAW_H__

