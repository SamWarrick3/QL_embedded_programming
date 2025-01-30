#include "circle.h"

Circle::Circle(int16_t x0, int16_t y0,
										 int16_t r0)
: Shape(x0, y0), /* base class ctor */

	r(r0)
{
}

void Circle::draw() const {
	//pass
}

uint32_t Circle::area() const {
	return 3U * (uint32_t)r * (uint32_t)r;
}