#ifndef __SHAPE_C__
#define __SHAPE_C__

#include "shape.h"

static void Shape_draw(Shape const * const me);
static uint32_t Shape_area(Shape const * const me);

void Shape_ctor(Shape * const me, int16_t x0, int16_t y0){
	static const struct ShapeVtable vtable = {
		&Shape_draw,
		&Shape_area
	};
	me->vptr = &vtable;
	me->x = x0;
	me->y = y0;
}	/* Shape constructor */

void Shape_moveBy(Shape * const me, int16_t x0, int16_t y0) {
	me->x += x0;
	me->y += y0;
}

uint16_t Shape_distanceTo(Shape const * const me, Shape * const other) {
	int16_t dx = me->x - other->x;
	int16_t dy = me->y - other->y;
	
	if (dx < 0) {
		dx = -dx;
	}
	
	if (dy < 0) {
		dy = -dy;
	
	}
	return dx + dy;
}

static void Shape_draw(Shape const * const me) {
	(void)me;
}

static uint32_t Shape_area(Shape const * const me) {
	(void)me;
	return 0U;
}

void drawGraph(Shape const *graph[]) {
	uint8_t idx = 0U;
	while (graph[idx] != (Shape *)0) {
		Shape_draw_vcall(graph[idx]);
		idx++;
	}
}

#endif // __SHAPE_C__