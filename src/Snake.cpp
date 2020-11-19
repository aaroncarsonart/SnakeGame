/*
 * Snake.cpp
 *
 *  Created on: Nov 16, 2020
 *      Author: aaron
 */

#include "Snake.h"

/**
 * Get the string name of this Direction.
 * @param d The Direction to get the name for.
 * @return The name of the input Direction.
 */
std::string direction_name(Direction direction) {
	switch (direction) {
	case Direction::UP:
		return "UP";
	case Direction::DOWN:
		return "DOWN";
	case Direction::LEFT:
		return "LEFT";
	case Direction::RIGHT:
		return "RIGHT";
	default:
		return "(none)";
	}
}

/**
 * Index used to differentiate different Segments.
 */
int segment_index = 1;

Segment::Segment() {
	// It doesn't matter if this value rolls over,
	// it only needs to be unique per "alive" Segments in Snake.
	// Snake.segment_count will never be greater than ROWS * COLS.
	index = segment_index++;
}

/**
 * Get a string representation of this Segment's fields and references.
 * @return A string describing this Segment.
 */
std::string Segment::to_string() {
	std::string str = "{index: " + std::to_string(index) + ", next: "
	        + (next == nullptr ? "nullptr" : std::to_string(next->index)) + ", prev: "
	        + (prev == nullptr ? "nullptr" : std::to_string(prev->index)) + ", p: "
	        + point->to_string() + "}";
	return str;
}

/**
 * Get the next Point towards which the Snake should move,
 * based on the Snake's current Direction.
 * @return The next Point towards which the Snake should move.
 */
Point* Snake::get_next_move() {
	int nx = 0;
	int ny = 0;
	Direction direction = this->direction;
	switch (direction) {
	case Direction::UP:
		nx = 0;
		ny = -1;
		break;
	case Direction::DOWN:
		nx = 0;
		ny = 1;
		break;
	case Direction::LEFT:
		nx = -1;
		ny = 0;
		break;
	case Direction::RIGHT:
		nx = 1;
		ny = 0;
		break;
	}
	nx += head->x();
	ny += head->y();
	return new Point(nx, ny);
}

/**
 * Grow the Snake one additional Segment towards the input Point.
 * @param point The Point towards which to grow the Snake.
 */
void Snake::grow(Point* point) {
	Segment* new_head = new Segment();
	new_head->point = point;

	// set head == tail
	if (segment_count == 0) {
		new_head->prev = nullptr;
		new_head->next = nullptr;
		head = new_head;
		tail = new_head;
	}
	// decouple head and tail
	else if (segment_count == 1) {
		new_head->prev = nullptr;
		new_head->next = tail;
		new_head->next->prev = new_head;
		new_head->next->next = nullptr;
		head = new_head;
	}
	// general case movement
	else {
		new_head->prev = nullptr;
		new_head->next = head;
		new_head->next->prev = new_head;
		head = new_head;
	}
	segment_count++;
}

/**
 * Move the Snake towards the input Point.
 * @param point The Point towards which to move the Snake.
 */
void Snake::move(Point* point) {
	grow(point);
	segment_count--;

	Segment* old_tail = tail;
	tail = tail->prev;
	tail->next = nullptr;
	delete old_tail->point;
	delete old_tail;
}

/**
 * Check if any of the Snake's Segments contain the given Point.
 * @param p The Point to check.
 * @return true if the Snake's Segments contain the given Point,
 *         else false.
 */
bool Snake::contains(Point* point) {
	for (Segment* s = head; s != nullptr; s = s->next) {
		if (point->equals(s->point)) {
			return true;
		}
	}
	return false;
}
