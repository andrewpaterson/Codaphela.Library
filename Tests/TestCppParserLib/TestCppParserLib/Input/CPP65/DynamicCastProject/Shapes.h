#ifndef __SHAPES_H__
#define __SHAPES_H__


struct CShape { int mi; virtual int Sides(void) { return 0; } };
struct CSquare : CShape { virtual int Sides(void) { return 4; } };
struct CCircle : CShape { virtual int Sides(void) { return 1; } };


#endif // __SHAPES_H__
