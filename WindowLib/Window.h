#ifndef __WINDOW_H__
#define __WINDOW_H__
#include "BasicComponent.h"
#include "Canvas.h"
#include "NativeWindow.h"


class CWindow : public CBasicComponent
{
CONSTRUCTABLE(CWindow); 
protected:
	CChars			mszWindowTitle;

	CNativeWindow*	mpcNativeWindow;
	Ptr<CCanvas>	mpCanvas;

public:
			void			Init(const char* szWindowTitle, CNativeWindowFactory* pcWindowFactory);
			void			Class(void);
			void 			Free(void);

			bool			Save(CObjectWriter* pcFile) override;
			bool			Load(CObjectReader* pcFile) override;

			bool			Show(void);
			void			Paint(void);
	virtual void			Tick(int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds) =0;

	virtual	void			CanvasChanged(Ptr<CCanvas> pNewCanvas) =0;

			void			CreateCanvas(EColourFormat eFormat, int32 iWidth, int32 iHeight);
			void			DestroyCanvas(void);

			const char*		GetWindowTitle(void);
			Ptr<CCanvas>	GetCanvas(void);
};


#endif // __WINDOW_H__

