#ifndef __WINDOW_H__
#define __WINDOW_H__
#include "BaseLib/Constructable.h"
#include "NativeWindow.h"


class CWindow : public CConstructable
{
CONSTRUCTABLE(CWindow); 
protected:
	CNativeWindow*	mpcNativeWindow;
	int64			miTime;

public:
			void	Init(CNativeWindow* pcNativeWindow);
			void	Kill(void);

	virtual bool	Show(void);
	virtual void	Tick(int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds);
};


#endif // __WINDOW_H__

