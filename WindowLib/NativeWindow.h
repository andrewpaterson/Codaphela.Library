#ifndef __NATIVE_WINDOW_H__
#define __NATIVE_WINDOW_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/Chars.h"


class CNativeWindow : public CConstructable
{
CONSTRUCTABLE(CNativeWindow);
protected:
	CChars	mszWindowTitle;

public:
	void Init(const char* szWindowTitle);
	void Kill(void);
};


#endif // __NATIVE_WINDOW_H__


