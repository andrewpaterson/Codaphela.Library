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
	float32					mfOpacity;  //The colour's channels, kept for drawing anti-aliased edges.
	float32					mfRed;
	float32					mfGreen;
	float32					mfBlue;

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

	bool	DrawSemicircle(int x, int y, int iDiameter, int iStartAngle, int iEndAngle, bool bFilled);  //Angles in degrees, 0 right and 90 up, anticlockwise from start to end.

	bool	DrawCel(int x, int y, Ptr<CImageCel> pCel, CRectangle* pcClip = NULL);
	bool	DrawCel(int x, int y, int iWidth, int iHeight, Ptr<CImageCel> pCel, CRectangle* pcClip = NULL);

protected:
	void	AngleDirection(int iAngle, int64* piX, int64* piY);
	void	BlendPixel(uint8* puiDest, uint8* puiSource, int iCoverage);
};


#endif // __IMAGE_DRAW_H__

