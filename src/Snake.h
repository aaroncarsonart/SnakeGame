/*
 * Snake.h
 *
 *  Created on: Nov 16, 2020
 *      Author: aaron
 */

#ifndef SNAKE_H_
#define SNAKE_H_

#include <string>
#include <stdbool.h>
#include "Point.h"

/**
 * Represents the four basic directions.
 */
enum class Direction {
	UP, DOWN, LEFT, RIGHT
};

/**
 * Get the string name of this Direction.
 * @param d The Direction to get the name for.
 * @return The name of the input Direction.
 */
std::string direction_name(Direction d);

/**
 * Represents a single segment of the snake.
 */
struct Segment {
	Point* point = nullptr;
	Segment* next = nullptr;
	Segment* prev = nullptr;
	int index;
	Segment();

	/**
	 * Get a string representation of this Segment's fields and references.
	 * @return A string describing this Segment.
	 */
	std::string to_string();
	int x() {
		return point->x;
	}
	int y() {
		return point->y;
	}
};

/**
 * Hold information for the snake.
 */
struct Snake {
	Direction direction = Direction::RIGHT;
	Segment* head = nullptr;
	Segment* tail = nullptr;
	int segment_count = 0;
	bool pause = false;
	bool game_over = false;

	/**
	 * Get the next Point towards which the Snake should move,
	 * based on the Snake's current Direction.
	 * @return The next Point towards which the Snake should move.
	 */
	Point* get_next_move();

	/**
	 * Grow the Snake one additional Segment towards the input Point.
	 * @param point The Point towards which to grow the Snake.
	 */
	void grow(Point* point);

	/**
	 * Move the Snake towards the input Point.
	 * @param point The Point towards which to move the Snake.
	 */
	void move(Point* point);

	/**
	 * Check if any of the Snake's Segments contain the given Point.
	 * @param p The Point to check.
	 * @return true if the Snake's Segments contain the given Point,
	 *         else false.
	 */
	bool contains(Point* point);
};

#endif /* SNAKE_H_ */
