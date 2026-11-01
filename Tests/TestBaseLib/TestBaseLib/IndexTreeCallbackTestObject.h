#ifndef __INDEX_TREE_CALLBACK_TEST_OBJECT_H__
#define __INDEX_TREE_CALLBACK_TEST_OBJECT_H__


class CTestIndexTreeMemoryObject
{
public:
	int		mi1;
	int		mi2;
	int		mi3;
	int		mi4;

	void Init(int i1, int i2, int i3, int i4);
};


struct STestIndexTreeFileObject
{
	int		i2;
	int		i4;

	void Init(int i2, int i4);
};


#endif // __INDEX_TREE_CALLBACK_TEST_OBJECT_H__

