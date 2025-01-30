#ifndef __CIRCLE_H__
#define __CIRCLE_H__

#include "shape.h"

class Circle : public Shape { /* inherit from shape */
private: /* CIRCLE's attributes */
	/* This class specific attributes */
	int16_t r;

public:
	Circle(int16_t x0, int16_t y0, int16_t r0);
	virtual void draw() const;
	virtual uint32_t area() const;
};
#endif // __CIRCLE_H__