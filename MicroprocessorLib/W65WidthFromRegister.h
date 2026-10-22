#ifndef __WIDTH_FROM_REGISTER_H__
#define __WIDTH_FROM_REGISTER_H__


enum EWidthFromRegister
{
	WFR_M,
	WFR_XY,
};


class CW65WidthFromRegister
{
public:
	static char*	GetName(EWidthFromRegister e);
};


#endif // __WIDTH_FROM_REGISTER_H__

