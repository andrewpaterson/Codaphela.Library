#ifndef __HOLDER_H__
#define __HOLDER_H__


//The friend declares CThing in the namespace: the same symbol as its definition in Thing.h.
class CHolder
{
friend class CThing;
protected:
	int		miValue;

public:
	void	Set(int i);
	int		Get(void);
};


#endif // __HOLDER_H__
