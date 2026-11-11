#ifndef __SHAPES_H__
#define __SHAPES_H__


namespace Shapes
{
	extern int giCount;

	struct SBox
	{
		int	w;

		int			Area(void);
		static int	Sides(void);
	};

	enum EKind
	{
		Square = 3,
	};

	int Scale(int a);
	int Scale(char c);

	template<class T>
	T Twice(T a)
	{
		return a + a;
	}

	template<class T>
	struct TBox
	{
		T	t;

		T Get(void)
		{
			return t;
		}
	};

	namespace Inner
	{
		int Deep(void);
	}
}


namespace Other
{
	int Scale(int a);
	int Local(void);
}


#endif // __SHAPES_H__
