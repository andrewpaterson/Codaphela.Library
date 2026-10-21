#ifndef __GERBER_RASTERISER_H__
#define __GERBER_RASTERISER_H__
#include "SupportLib/CoverageRasteriser.h"
#include "GerberVectorImage.h"


//One layer as 8 bit opacity, 0 empty to 255 dark, y down.  The top left corner of the top left pixel is at msOrigin (mm).
class CGerberRaster
{
protected:
	uint8*			mpuiData;
	int				miWidth;
	int				miHeight;
	SFloat64Vec2	msOrigin;
	float64			mfPixelsPerMillimetre;

public:
	bool			Init(int iWidth, int iHeight, SFloat64Vec2* psOrigin, float64 fPixelsPerMillimetre);
	bool			Init(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, float64 fPixelsPerMillimetre);
	void			Kill(void);

	void			Clear(void);

	uint8*			GetData(void);
	int				GetWidth(void);
	int				GetHeight(void);
	uint8			Get(int x, int y);
	SFloat64Vec2*	GetOrigin(void);
	float64			GetPixelsPerMillimetre(void);
	float64			Area(void);
};


//Fills the objects of a vector image into a raster in order: dark objects darken it and clear objects lighten it.
//Arcs are flattened to lines about the tolerance (pixels) from the arc, either side of it so areas are kept.  The parts of an object are combined
//first; exposure on adds and exposure off erases.  All the coverage arithmetic is integer.
class CGerberRasteriser
{
protected:
	CCoverageRasteriser		mcCoverage;
	uint8*					mpuiObject;
	size					muiObjectCapacity;
	float64					mfTolerance;

	CGerberRaster*			mpcRaster;
	int						miLeft;
	int						miTop;

public:
	void	Init(float64 fTolerance = 0.05);
	void	Kill(void);

	bool	Rasterise(CGerberVectorImage* pcImage, CGerberRaster* pcRaster);

protected:
	bool	RasteriseObject(CGerberVectorObject* pcObject);
	bool	RasterisePart(CGerberVectorObject* pcObject, size uiStart, size uiEnd, int iWidth, int iHeight);
	void	AddArc(SFloat64Vec2* psStart, SFloat64Vec2* psEnd, SFloat64Vec2* psCenter, bool bClockwise);
	void	AddLine(SFloat64Vec2* psStart, SFloat64Vec2* psEnd);
	int32	ToSubpixelX(float64 x);
	int32	ToSubpixelY(float64 y);
	void	Composite(uint8* puiCoverage, int iWidth, int iHeight, bool bDark);
};


#endif // __GERBER_RASTERISER_H__

