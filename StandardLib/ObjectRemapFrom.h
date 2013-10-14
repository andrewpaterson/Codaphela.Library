#ifndef __OBJECT_REMAP_FROM_H__
#define __OBJECT_REMAP_FROM_H__


class CEmbeddedObject;
class CObjectRemapFrom
{
public:
	int Remap(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew);

private:
	int RemapEmbedded(CEmbeddedObject* pcNew, CEmbeddedObject* pcOld);
	int	CalculateNumEmbedded(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew);
};


#endif // __OBJECT_REMAP_FROM_H__

