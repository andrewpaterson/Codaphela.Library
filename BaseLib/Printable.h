#ifndef __PRINTABLE_H__
#define __PRINTABLE_H__


class CChars;
class CPrintable
{
public:
	virtual void Print(CChars* psz) =0;
};


#endif // __PRINTABLE_H__

