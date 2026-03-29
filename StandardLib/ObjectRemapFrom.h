#ifndef __OBJECT_REMAP_FROM_H__
#define __OBJECT_REMAP_FROM_H__


class CEmbeddedObject;
class CObjectRemapFrom
{
public:
	size	Remap(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew);

private:
	size	RemapEmbedded(CEmbeddedObject* pcNew, CEmbeddedObject* pcOld);
	size	CalculateNumEmbedded(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew);
};


#endif // __OBJECT_REMAP_FROM_H__

