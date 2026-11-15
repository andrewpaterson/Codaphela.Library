#ifndef __COUNTER_H__
#define __COUNTER_H__


//Two size policies, the shape of BaseLib's ArrayTemplatesPlan.txt: a count type, constants and a function.
struct SNarrow
{
	typedef short		Count;
	static const Count	Max = 100;
	static const bool	Check = true;

	static Count Clamp(int i) { return (Count)((i > Max) ? Max : i); }
};

struct SWide
{
	typedef int			Count;
	static const Count	Max = 1000;
	static const bool	Check = false;

	static Count Clamp(int i) { return (Count)i; }
};


template<int N>
struct SBuffer
{
	char	ac[N];
};


template<class SIZE>
class CCounter
{
protected:
	typename SIZE::Count		miCount;
	char						macBuffer[SIZE::Max / 10];
	SBuffer<SIZE::Max / 50>		msSpare;

public:
	void					Init(void) { miCount = 0; }
	void					Add(int i);
	typename SIZE::Count	Get(void);
	int						BufferSize(void);
	virtual int				Kind(void);
};


template<class SIZE>
void CCounter<SIZE>::Add(int i)
{
	if (SIZE::Check)
	{
		miCount = SIZE::Clamp(miCount + i);
	}
	else
	{
		miCount = (typename SIZE::Count)(miCount + i);
	}
}


template<class SIZE>
typename SIZE::Count CCounter<SIZE>::Get(void)
{
	return miCount;
}


template<class SIZE>
int CCounter<SIZE>::BufferSize(void)
{
	static int	siLimit = SIZE::Max;

	return (int)sizeof(macBuffer) + (int)sizeof(msSpare) + siLimit;
}


template<class SIZE>
int CCounter<SIZE>::Kind(void)
{
	switch (SIZE::Max)
	{
	case SNarrow::Max:
		return 1;
	case SWide::Max:
		return 2;
	}
	return 0;
}


//A member of a base that depends on a template parameter is named through this->.
template<class M, class SIZE>
class CTally : public CCounter<SIZE>
{
public:
	M		mTotal;

	void	Tally(M m);
};


template<class M, class SIZE>
void CTally<M, SIZE>::Tally(M m)
{
	mTotal += m;
	this->Add(1);
	this->miCount += 0;
}


template<class T>
T Twice(T t)
{
	return t + t;
}


//Instances.cpp instantiates them; no other unit generates their functions.
extern template class CCounter<SNarrow>;
extern template class CCounter<SWide>;
extern template class CTally<int, SNarrow>;
extern template int Twice<int>(int);
extern template short Twice(short);


#endif // __COUNTER_H__

