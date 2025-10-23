#ifndef __NATIVE_WINDOW_H__
#define __NATIVE_WINDOW_H__
#include "BaseLib/Chars.h"
#include "BasicNativeComponent.h"


class CNativeWindowFactory;
class CNativeWindow : public CBasicNativeComponent
{
CONSTRUCTABLE(CNativeWindow);
friend class CWindow;
protected:
	CWindow*	mpcWindow;

public:
			void	Init(CWindow* pcWindow, CNativeWindowFactory* pcWindowFactory);
			void	Kill(void);

protected:
	virtual bool	CreateNativeWindow(void) =0;
	virtual bool	ExecuteNativeWindow(void) =0;
	virtual void	PaintNativeWindow(void) =0;
};


#endif // __NATIVE_WINDOW_H__


