#include "rectangle.h"

Rectangle::Rectangle(int16_t x0, int16_t y0,
										 int16_t w0, int16_t h0)
: Shape(x0, y0), /* base class ctor */
																						
	w(w0),
	h(h0)
{
}

void Rectangle::draw() const {
	//pass
}

uint32_t Rectangle::area() const {
	return (uint32_t)w * (uint32_t)h;
}