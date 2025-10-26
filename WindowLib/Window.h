#ifndef __WINDOW_H__
#define __WINDOW_H__
#include "Component.h"
#include "Canvas.h"
#include "NativeWindow.h"


class CWindow : public CComponent
{
CONSTRUCTABLE(CWindow); 
protected:
	CChars			mszWindowTitle;

	CNativeWindow*	mpcNativeWindow;
	CCanvas			mcCanvas;

public:
			void			Init(const char* szWindowTitle, CNativeWindowFactory* pcWindowFactory);
			void			Kill(void);

			bool			Show(void);
			void			Paint(void);
	virtual void			Tick(int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds) =0;

	virtual	void			CanvasChanged(CCanvas* pcNewCanvas) =0;

			void			CreateCanvas(EColourFormat eFormat, int32 iWidth, int32 iHeight);
			void			DestroyCanvas(void);

			const char*		GetWindowTitle(void);
			CCanvas*		GetCanvas(void);
};


#endif // __WINDOW_H__

