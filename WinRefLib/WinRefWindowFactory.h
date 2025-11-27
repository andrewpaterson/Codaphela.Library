#ifndef __WIN_Ref_WINDOW_FACTORY_H__
#define __WIN_Ref_WINDOW_FACTORY_H__
#include "BaseLib/Define.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/Chars.h"
#include "SupportLib/ColourFormat.h"
#include "WindowLib/NativeWindowFactory.h"


class CWinRefWindowFactory : public CNativeWindowFactory
{
CONSTRUCTABLE(CWinRefWindowFactory);
protected:
	CRectangle	mcBounds;
	CChars		mszImagePath;

public:
	void			Init(CMallocator* pcMallocator, int32 iWidth, int32 iHeight, char* szImagePath);
	void			Kill(void);

	CNativeWindow*	CreateNativeWindow(CWindow* pcWindow) override;
	CNativeCanvas*	CreateNativeCanvas(CCanvas* pcCanvas) override;

protected:

};


#endif // __WIN_Ref_WINDOW_FACTORY_H__

