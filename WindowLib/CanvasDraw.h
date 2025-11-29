#ifndef __CANVAS_DRAW_H__
#define __CANVAS_DRAW_H__
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"


/* The only import method in this class is

Draw()

Everything else just exists as a useful template to copy/paste objects from.

*/


class CCanvas;
class CCanvasDraw : public CObject
{
CONSTRUCTABLE(CCanvasDraw);
DESTRUCTABLE(CCanvasDraw);
protected:
public:
			void	Init(void);
			void	Class(void) override;
			void 	Free(void) override;

			bool	Save(CObjectWriter* pcFile) override;
			bool	Load(CObjectReader* pcFile) override;

	virtual bool	Draw(Ptr<CCanvas> pCanvas) =0;
};


#endif // __CANVAS_DRAW_H__

