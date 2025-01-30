#ifndef __RECTANGLE_H__
#define __RECTANGLE_H__

#include "shape.h"


class Rectangle : public Shape { /* inherit from shape */
private: /* Rectangle's attributes */
	/* This class specific attributes */
	int16_t w;
	int16_t h;

public:
	Rectangle(int16_t x0, int16_t y0,
																						int16_t w0, int16_t h0);
	virtual void draw() const;
	virtual uint32_t area() const;
};
#endif // __RECTANGE_H__