#include "rectangle.h"

void Rectangle_ctor(Rectangle * const me, int16_t x0, int16_t y0,
																					int16_t w0, int16_t h0) {
	static const struct ShapeVtable vtable = {
		(void (*)(Shape const * const me))Rectangle_draw, /* cast to function in ShapeVtable definition */
		(uint32_t (*)(Shape const * const me))Rectangle_area
	};
	
	Shape_ctor(&me->super, x0, y0); /* base class ctor */
	me->super.vptr = &vtable;
	me->w = w0;
	me->h = h0;
}

void Rectangle_draw(Rectangle const * const me) {
	//pass
}

uint32_t Rectangle_area(Rectangle const * const me) {
	return (uint32_t)me->w * (uint32_t)me->h;
}