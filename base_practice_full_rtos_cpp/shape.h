#ifndef __SHAPE_H__
#define __SHAPE_H__

#include <stdint.h>

class Shape {
protected:
	int16_t x; /* x-coordinate of Shape's position */
	int16_t y; /* y-coordinate of Shape's position */

public: /* in c++ all operations take an implicit 'this' Shape object */
	Shape(int16_t x0, int16_t y0); /* Shape constructor */
	void moveBy(int16_t x0, int16_t y0);
	uint16_t distanceTo(Shape * const other) const;

	virtual void draw() const;
	virtual uint32_t area() const;
};

void drawGraph(Shape const *graph[]);

#endif // __SHAPE_H__