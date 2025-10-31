#ifndef __IMAGE_DRAW_H__
#define __IMAGE_DRAW_H__
#include "StandardLib/Pointer.h"
#include "Image.h"
#include "ImageAccessor.h"
#include "ImageColour.h"


class CImageDraw
{
protected:
	Ptr<CImage>			mpImage;
	CImageAccessor*		mpcAccessor;
	SImageColour		msColour;

public:
	void	Init(Ptr<CImage> pImage);
	void	Kill(void);

	void	SetColour(CImageColour* pcColour);

	void	DrawBox(CRectangle* pcRetangle, bool bFilled);
	void	DrawBox(bool bFilled);
};


#endif // __IMAGE_DRAW_H__

