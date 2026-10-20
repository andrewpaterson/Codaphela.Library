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
	Ptr<CImage>                 mpDestImage;
	bool						mbClear;  //Make the image black before the maps are blitted onto it.

public:
	void	Init(Ptr<CMaps> pMaps);
	void	SetClear(bool bClear);
	void	Class(void) override;
	void 	Free(void) override;

	bool	Save(CObjectWriter* pcFile) override;
	bool	Load(CObjectReader* pcFile) override;

	bool	Draw(Ptr<CCanvas> pCanvas) override;
};


#endif // __MAPS_CANVAS_DRAW_H__

