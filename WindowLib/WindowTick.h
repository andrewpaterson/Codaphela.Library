#ifndef __WINDOW_TICK_H__
#define __WINDOW_TICK_H__
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"


/* The only import method in this class is

Tick()

Everything else just exists as a useful template to copy/paste objects from.

*/


class CWindow;
class CWindowTick : public CObject
{
CONSTRUCTABLE(CWindowTick);
protected:
public:
			void	Init(void);
			void	Class(void) override;
			void 	Free(void) override;

			bool	Save(CObjectWriter* pcFile) override;
			bool	Load(CObjectReader* pcFile) override;

	virtual void	Tick(Ptr<CWindow> pWindow, int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds) =0;
};


#endif // __WINDOW_TICK_H__

