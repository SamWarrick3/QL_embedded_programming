#ifndef __SHAPE_H__
#define __SHAPE_H__

#include <stdint.h>

typedef struct {
	struct ShapeVtable const *vptr; /* virtual pointer */
	
	int16_t x; /* x-coordinate of Shape's position */
	int16_t y; /* y-coordinate of Shape's position */
} Shape;

struct ShapeVtable {
	void (*draw)(Shape const * const me); /* pointer to function, pointer name is draw */
	uint32_t (*area)(Shape const * const me);
};

void Shape_ctor(Shape * const me, int16_t x0, int16_t y0); /* Shape constructor */
void Shape_moveBy(Shape * const me, int16_t x0, int16_t y0);
uint16_t Shape_distanceTo(Shape const * const me, Shape * const other);

static inline void Shape_draw_vcall(Shape const * const me) {
	(*me->vptr->draw)(me);
}

static inline uint32_t Shape_area_vcall(Shape const * const me) {
	return (*me->vptr->area)(me);
}

void drawGraph(Shape const *graph[]);

#endif // __SHAPE_H__