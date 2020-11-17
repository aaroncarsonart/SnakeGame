/*
 * Point.h
 *
 *  Created on: Nov 14, 2020
 *      Author: aaron
 */

#ifndef POINT_H_
#define POINT_H_
#include <stdbool.h>
#include <string>

struct Point {
	int x = 0;
	int y = 0;
	/**
	 * Create a new 2D Point with integer coordinates.
	 * @param x The x coordinate to use.
	 * @param y The y coordinate to use.
	 */
	Point(int x, int y) {
		this->x = x;
		this->y = y;
	}
	Point() {}

	/**
	 * Check if the this Point matches the given Point.
	 * @param p The Point to check against.
	 * @return True if the two Points are equivalent.
	 */
	bool equals(Point* p) {
		return this->x == p->x && this->y == p->y;
	}

	/**
	 * return a string representation of this Point.
	 * @return a string representation of this Point.
	 */
	std::string to_string() {
		return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
	}
};

#endif /* POINT_H_ */
