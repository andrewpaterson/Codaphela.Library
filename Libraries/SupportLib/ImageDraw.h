#ifndef __IMAGE_DRAW_H__
#define __IMAGE_DRAW_H__
#include "StandardLib/Pointer.h"
#include "Image.h"
#include "ImageAccessor.h"
#include "ImageColour.h"
#include "ImageBlitter.h"
#include "ImageRowBlitterFactory.h"
#include "VectorRasteriser.h"
#include "VectorShapes.h"


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
	CVectorRasteriser		mcRasteriser;

public:
	void	Init(CImage* ppImage);  //A (*) pointer instead of Ptr pointer because CImageDraw is not an object.  Note that the Image should be added into FreeObjects to ensure it's not evicted whilst this pointer is in use.
	void	Kill(void);

	void	SetColour(CImageColour* pcColour);

	void	DrawPixel(int x, int y);

	void	DrawBox(CInt32Rect* pcRetangle, bool bFilled);
	void	DrawBox(int iLeft, int iTop, int iRight, int iBottom, bool bFilled);
	void	DrawBox(bool bFilled);

	void	DrawLine(int x0, int y0, int x1, int y1);
	void	DrawHorizontalLine(int x0, int x1, int y);
	void	DrawVerticalLine(int x, int y0, int y1);

	bool	DrawSemicircle(int x, int y, int iDiameter, int iStartAngle, int iEndAngle, bool bFilled);  //Angles in degrees, 0 right and 90 up, anticlockwise from start to end.

	//Anti-aliased shapes.  Positions are pixels from the image's top left, y down; pixel x, y covers x to x + 1.
	//Angles are degrees, 0 right and 90 up, anticlockwise from start to end.  The image must have opacity.
	bool	DrawPath(CVectorPath* pcPath);
	bool	DrawPath(CVectorPath* pcPath, SFloat64Vec2* psOrigin, float64 fPixelsPerUnit, bool bYUp);
	bool	DrawCircle(float64 x, float64 y, float64 fRadius);
	bool	DrawSector(float64 x, float64 y, float64 fRadius, float64 fStartAngle, float64 fEndAngle);
	bool	DrawRing(float64 x, float64 y, float64 fOuterRadius, float64 fInnerRadius, float64 fStartAngle, float64 fEndAngle);
	bool	DrawStroke(float64 x0, float64 y0, float64 x1, float64 y1, float64 fWidth);
	bool	DrawPolygon(SFloat64Vec2* asPoints, size uiNumPoints);
	bool	DrawRectangle(float64 fLeft, float64 fTop, float64 fRight, float64 fBottom);
	bool	DrawRoundedRectangle(float64 fLeft, float64 fTop, float64 fRight, float64 fBottom, float64 fRadius);
	bool	DrawRoundedRectangle(float64 fLeft, float64 fTop, float64 fRight, float64 fBottom, float64 fTopRadius, float64 fBottomRadius);

	bool	DrawCel(int x, int y, Ptr<CImageCel> pCel, CInt32Rect* pcClip = NULL);
	bool	DrawCel(int x, int y, int iWidth, int iHeight, Ptr<CImageCel> pCel, CInt32Rect* pcClip = NULL);

protected:
	bool	DrawCentredPath(CVectorPath* pcPath, float64 x, float64 y);
	bool	BlendCoverage(void);
};


#endif // __IMAGE_DRAW_H__

