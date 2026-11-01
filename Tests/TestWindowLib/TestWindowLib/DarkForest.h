#ifndef __DARK_FOREST_H__
#define __DARK_FOREST_H__
#include "StandardLib/Pointer.h"
#include "SupportLib/ColourARGB32.h"
#include "SupportLib/ColourFormat.h"
#include "SupportLib/ImageCel.h"
#include "WindowLib/CanvasDraw.h"
#include "WindowLib/ContainerStyle.h"


struct SDarkForestComponent
{
	const char*	szImage;		//NULL for a border canvas.
	SInt32Vec2	sDesiredSize;	//-1, -1 lets an image canvas take the size of its cel.
	ARGB32		sBorderColour;

	EContainerStyleHorizontal	eHorizontal;	//Unknown on both axes leaves the canvas's default alignment.
	EContainerStyleVertical		eVertical;
};


Ptr<CImageCel>		ReadDarkForestCel(const char* szFilename, CColourFormatHelper* pcFormat);
Ptr<CCanvasDraw>	CreateDarkForestDraw(SDarkForestComponent* psComponent, CColourFormatHelper* pcFormat);


#endif // __DARK_FOREST_H__

