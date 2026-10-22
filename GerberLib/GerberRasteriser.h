#ifndef __GERBER_RASTERISER_H__
#define __GERBER_RASTERISER_H__
#include "SupportLib/VectorRasteriser.h"
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
//Each object is filled by a vector rasteriser; arcs are flattened to lines about the tolerance (pixels) from the arc.
class CGerberRasteriser
{
protected:
	CVectorRasteriser		mcVector;
	CGerberRaster*			mpcRaster;

public:
	void	Init(float64 fTolerance = 0.05);
	void	Kill(void);

	bool	Rasterise(CGerberVectorImage* pcImage, CGerberRaster* pcRaster);

protected:
	bool	RasteriseObject(CGerberVectorObject* pcObject);
	void	Composite(bool bDark);
};


#endif // __GERBER_RASTERISER_H__

