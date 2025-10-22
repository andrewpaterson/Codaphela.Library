#ifndef __NATIVE_WINDOW_H__
#define __NATIVE_WINDOW_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/Chars.h"


class CNativeWindowFactory;
class CNativeWindow : public CConstructable
{
CONSTRUCTABLE(CNativeWindow);
friend class CWindow;
protected:
	CChars					mszWindowTitle;
	CNativeWindowFactory*	mpcWindowFactory;

public:
			void					Init(const char* szWindowTitle, CNativeWindowFactory* pcWindowFactory);
			void					Kill(void);

			CNativeWindowFactory*	GetFactory(void);

protected:
	virtual bool	CreateNativeWindow(void) =0;
	virtual bool	ExecuteNativeWindow(void) =0;
	virtual void	PaintNativeWindow(void) =0;
};


#endif // __NATIVE_WINDOW_H__


