#ifndef __RECTANGLE_H__
#define __RECTANGLE_H__

#include "shape.h"

/* Rectangle's attributes */
typedef struct {
	Shape super; /* inherit from shape */
	/* This class specific attributes */
	int16_t w;
	int16_t h;
} Rectangle;

void Rectangle_ctor(Rectangle * const me, int16_t x0, int16_t y0,
																					int16_t w0, int16_t h0);
void Rectangle_draw(Rectangle const * const me);
uint32_t Rectangle_area(Rectangle const * const me);

#endif // __RECTANGE_H__