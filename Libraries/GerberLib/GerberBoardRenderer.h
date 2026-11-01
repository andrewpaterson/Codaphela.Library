#ifndef __GERBER_BOARD_RENDERER_H__
#define __GERBER_BOARD_RENDERER_H__
#include "BaseLib/DataTypes.h"
#include "StandardLib/Pointer.h"
#include "SupportLib/Image.h"
#include "GerberRasteriser.h"
#include "GerberFileSet.h"


struct SGerberRenderColour
{
	uint8	r;
	uint8	g;
	uint8	b;
};


//Draws one side of a board, or a panel, as it looks when made: the board shape cut from the edge cuts, then from the
//inside out the copper, the solder mask and the silkscreen, then the drill holes.  The mask is translucent, so
//copper under it shows lighter green, and copper in a mask opening shows as gold (ENIG).  The holes show the
//background through.  Paste is left out; it is the stencil for assembly, not part of the board.  The back is mirrored
//left to right, as seen after turning the board over.
class CGerberBoardRenderer
{
protected:
	float64				mfPixelsPerMillimetre;
	float64				mfMargin;
	SGerberRenderColour	msBackground;
	SGerberRenderColour	msSubstrate;
	SGerberRenderColour	msCopper;
	SGerberRenderColour	msFinish;
	SGerberRenderColour	msMask;
	float64				mfMaskOpacity;
	SGerberRenderColour	msSilk;

public:
	void		Init(float64 fPixelsPerMillimetre = 20.0, float64 fMargin = 2.0);
	void		Kill(void);

	Ptr<CImage>	Render(CGerberFileSet* pcSet, EGerberBoardSide eSide);

protected:
	bool		DrawShape(CGerberFileSet* pcSet, CGerberRaster* pcRaster);
	bool		DrawLayer(CGerberFileSet* pcSet, EGerberBoardLayer eLayer, EGerberBoardSide eSide, CGerberRaster* pcRaster);
	bool		DrawDrills(CGerberFileSet* pcSet, CGerberRaster* pcRaster);
	bool		Rasterise(CGerberVectorImage* pcImage, CGerberRaster* pcRaster);
	void		Blend(float64* afColour, SGerberRenderColour* psColour, float64 fAmount);
	void		SetColour(SGerberRenderColour* psColour, uint8 r, uint8 g, uint8 b);
};


#endif // __GERBER_BOARD_RENDERER_H__
