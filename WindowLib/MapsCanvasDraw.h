#ifndef __MAPS_CANVAS_DRAW_H__
#define __MAPS_CANVAS_DRAW_H__
#include "SupportLib/Maps.h"
#include "WindowLib/CanvasDraw.h"


class CMapsCanvasDraw : public CCanvasDraw
{
CONSTRUCTABLE(CMapsCanvasDraw);
DESTRUCTABLE(CMapsCanvasDraw);
protected:
	Ptr<CMaps>	mpMaps;

public:
			void	Init(Ptr<CMaps> pMaps);
			void	Class(void) override;
			void 	Free(void) override;

			bool	Save(CObjectWriter* pcFile) override;
			bool	Load(CObjectReader* pcFile) override;

	virtual bool	Draw(Ptr<CCanvas> pCanvas) =0;
};


#endif // __MAPS_CANVAS_DRAW_H__

