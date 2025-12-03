#ifndef __WINDOW_H__
#define __WINDOW_H__
#include "ComplexComponent.h"
#include "Canvas.h"
#include "NativeWindow.h"
#include "CanvasDraw.h"


class CWindowTick;
class CWindow : public CComplexComponent
{
CONSTRUCTABLE(CWindow); 
DESTRUCTABLE(CWindow);
protected:
	CChars				mszWindowTitle;

	CNativeWindow*		mpcNativeWindow;
	Ptr<CCanvas>		mpCanvas;
	Ptr<CWindowTick>	mpWindowTick;
	bool				mbTicking;
	CFocus				mFocus;
	CRectangle			mcClientRect;

public:
	void				Init(const char* szTitle, CNativeWindowFactory* pcFactory, Ptr<CWindowTick>	pTick, Ptr<CCanvasDraw> pDraw);
	void				Class(void);
	void 				Free(void);
	
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;
	
	bool				Show(void);
	void				Paint(void);
	void				Tick(int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds);
	void				Stop(void);
	void				Layout(void);
	
	void				CreateCanvas(EColourFormat eFormat, int32 iWidth, int32 iHeight);
	
	const char*			GetWindowTitle(void);
	Ptr<CCanvas>		GetCanvas(void);
	Ptr<CFocus>			GetFocus(void);
	CRectangle*			GetClientRect(void);

	bool				SetContainer(Ptr<CContainer> pContainer);
	Ptr<CContainer>		GetContainer(void);
	bool				ClearContainer(void);
};


#endif // __WINDOW_H__

