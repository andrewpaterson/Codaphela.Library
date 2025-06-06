#ifndef __KILLABLE_H__
#define __KILLABLE_H__


class CChars;
class CKillable
{
public:
	virtual void Kill(void) =0;
	virtual void Print(CChars* psz);
};


#endif // __KILLABLE_H__

