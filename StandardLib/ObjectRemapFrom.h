#ifndef __OBJECT_REMAP_FROM_H__
#define __OBJECT_REMAP_FROM_H__


class CEmbeddedObject;
class CObjectRemapFrom
{
public:
	int Remap(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew);

private:
	int RemapEmbedded(CEmbeddedObject* pcNew, CEmbeddedObject* pcOld);
};


#endif // __OBJECT_REMAP_FROM_H__

