#ifndef __KILLABLE_H__
#define __KILLABLE_H__


class CChars;
class CKillable
{
public:
	virtual void Kill(void) =0;
	virtual void Print(CChars* psz);  //Killable includes Print because having a separate CPrintable changes the size of pointers to inheriting objects.
};


#endif // __KILLABLE_H__

