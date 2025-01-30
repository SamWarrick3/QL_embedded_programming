#ifndef __SHAPE_C__
#define __SHAPE_C__

#include "shape.h"

Shape::Shape(int16_t x0, int16_t y0)
:
	x(x0),
	y(y0)
{
}	/* Shape constructor usiong c++ constructor list format */

void Shape::moveBy(int16_t x0, int16_t y0) {
	this->x += x0;
	this->y += y0;
}

uint16_t Shape::distanceTo(Shape * const other) const {
	int16_t dx = x - other->x; /* c++ will implicitly recognize class attributes */
	int16_t dy = y - other->y; /* so no need for 'this' keyword */
	
	if (dx < 0) {
		dx = -dx;
	}
	
	if (dy < 0) {
		dy = -dy;
	
	}
	return dx + dy;
}

void Shape::draw() const {

}

uint32_t Shape::area() const {
	return 0U;
}

void drawGraph(Shape const *graph[]) {
	uint8_t idx = 0U;
	while (graph[idx] != (Shape *)0) {
		graph[idx]->draw(); /* polymorphism, calls draw method attached to the obj type */
		idx++;
	}
}
#endif // __SHAPE_C__