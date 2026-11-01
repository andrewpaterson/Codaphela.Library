#ifndef __INPUT_TEST_COMPONENT_H__
#define __INPUT_TEST_COMPONENT_H__
#include "BaseLib/Chars.h"
#include "WindowLib/Component.h"


//Appends every focus and input call it is given to a log shared by all the
//components in a test, as "Name:Call;", so the order of calls can be asserted.
class CInputTestComponent : public CComponent
{
CONSTRUCTABLE(CInputTestComponent);
DESTRUCTABLE(CInputTestComponent);
protected:
	CChars		mszName;
	CChars*		mpszLog;

public:
	void	Init(Ptr<CWindow> pWindow, const char* szName, CChars* pszLog, bool bCanGetFocus);
	void	Class(void) override;
	void	Free(void) override;

	void	FocusGained(void) override;
	void	FocusLost(void) override;
	void	MovedInto(void) override;
	void	MovedOutof(void) override;

	void	CommandAction(SComponentInputEvent* psEvent);
	void	PrintableAction(SComponentInputEvent* psEvent);
	void	WheelAction(SComponentInputEvent* psEvent);

protected:
	void	Log(const char* szCall);
	void	Log(const char* szCall, int x, int y);
};


#endif // __INPUT_TEST_COMPONENT_H__

