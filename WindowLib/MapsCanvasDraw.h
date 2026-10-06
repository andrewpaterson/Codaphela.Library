#ifndef __MAPS_CANVAS_DRAW_H__
#define __MAPS_CANVAS_DRAW_H__
#include "SupportLib/Maps.h"
#include "WindowLib/CanvasDraw.h"


class CMapsCanvasDraw : public CCanvasDraw
{
CONSTRUCTABLE(CMapsCanvasDraw);
DESTRUCTABLE(CMapsCanvasDraw);
protected:
	Ptr<CMaps>					mpMaps;
	Ptr<CImageCelBlitterCache>	mpBlitterCache;
	Ptr<CImage>                 mpDestImage;

public:
	void	Init(Ptr<CMaps> pMaps);
	void	Class(void) override;
	void 	Free(void) override;

	bool	Save(CObjectWriter* pcFile) override;
	bool	Load(CObjectReader* pcFile) override;

	bool	Draw(Ptr<CCanvas> pCanvas) override;
};


#endif // __MAPS_CANVAS_DRAW_H__

