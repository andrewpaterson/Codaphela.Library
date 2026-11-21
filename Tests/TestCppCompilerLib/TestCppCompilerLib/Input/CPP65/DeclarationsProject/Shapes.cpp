//The shapes' functions: each class it defines a member of is needed whole.
#include "Shapes.h"


int CShape::Area(void) { return 0; }
int CShape::Sides(void) { return 0; }
int CSquare::Area(void) { return miSize * miSize; }
int CSquare::Sides(void) { return 4; }
int CTriangle::Area(void) { return (miSize * miSize) / 2; }
int CTriangle::Sides(void) { return 3; }
int CCircle::Area(void) { return miSize * miSize * 3; }
int CCircle::Sides(void) { return 1; }
int CHexagon::Area(void) { return miSize * 6; }
int CHexagon::Sides(void) { return 6; }
int COctagon::Area(void) { return miSize * 8; }
int COctagon::Sides(void) { return 8; }
int CCube::Area(void) { return miSize * miSize * 6; }
int CPrism::Area(void) { return miSize * 5; }
int CBall::Area(void) { return miSize * miSize * 12; }
