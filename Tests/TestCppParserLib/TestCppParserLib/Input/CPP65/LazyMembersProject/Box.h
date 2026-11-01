#ifndef __BOX_H__
#define __BOX_H__


//Members defined outside their classes: parsed after the class, so not at all in a unit that never names it.
template<class M>
class CBox
{
protected:
	M	mValue;

public:
	void	Set(M value);
	M		Get(void);
	M		Add(M value);
};


template<class M>
void CBox<M>::Set(M value)
{
	mValue = value;
}


template<class M>
M CBox<M>::Get(void)
{
	return mValue;
}


template<class M>
M CBox<M>::Add(M value)
{
	return mValue + value;
}


class CCounter
{
protected:
	int		miCount;

public:
	void	Init(void);
	int		Next(void);
};


inline void CCounter::Init(void)
{
	miCount = 0;
}


inline int CCounter::Next(void)
{
	miCount++;
	return miCount;
}


template<class M>
class CUnusedBox
{
public:
	M	Never(void);
};


template<class M>
M CUnusedBox<M>::Never(void)
{
	return (M)9;
}


#endif // __BOX_H__
