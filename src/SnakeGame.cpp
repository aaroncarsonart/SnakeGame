//============================================================================
// Name        : SnakeGame.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : The classic SnakeGame, implemented in C++ for the terminal.
//============================================================================

#include <iostream>
#include <string>
#include <random>

#include <ncurses.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#include "Point.h"
#include "Keys.h"
#include "Snake.h"

/**
 * Read the user input.  Intended to be run on a separate thread
 * via passing as an argument to {@code pthread_create}.
 * @param vargp The Snake to be updated by the user input.
 */
void* read_user_input(void* vargp) {
	Snake* snake = (Snake*) vargp;
	while (true) {
		int input = getch();
		switch (input) {
		case A_KEY_A:
		case A_KEY_H:
		case A_KEY_LEFT:
		case KEY_LEFT:
			if (snake->direction != LEFT && snake->direction != RIGHT) {
				snake->direction = LEFT;
				snake->pause = false;
			}
			break;
		case A_KEY_S:
		case A_KEY_J:
		case A_KEY_DOWN:
		case KEY_DOWN:
			if (snake->direction != DOWN && snake->direction != UP) {
				snake->direction = DOWN;
				snake->pause = false;
			}
			break;
		case A_KEY_W:
		case A_KEY_K:
		case A_KEY_UP:
		case KEY_UP:
			if (snake->direction != UP && snake->direction != DOWN) {
				snake->direction = UP;
				snake->pause = false;
			}
			break;
		case A_KEY_D:
		case A_KEY_L:
		case A_KEY_RIGHT:
		case KEY_RIGHT:
			if (snake->direction != RIGHT && snake->direction != LEFT) {
				snake->direction = RIGHT;
				snake->pause = false;
			}
			break;
		case A_KEY_P:
		case A_KEY_SPACE:
			snake->pause = !snake->pause;
			break;
		case A_KEY_Q:
		case A_KEY_ESCAPE:
			endwin();
			exit(0);
			break;
		}
	}
	return NULL;
}

/**
 * Variables used for RNG.
 */
std::default_random_engine random_engine;
std::uniform_int_distribution<int> width_dist;
std::uniform_int_distribution<int> height_dist;

/**
 * Get a random Point within the range of ROWS and COLS.
 * @return a random Point within the range of ROWS and COLS.
 */
Point* get_random_point() {
	int x = width_dist(random_engine);
	int y = height_dist(random_engine);
	return new Point(x, y);
}

/**
 * Enum used for color definitions with ncurses.
 */
enum Color {
	RED     = 1,
	GREEN   = 2,
	BLUE    = 3,
	YELLOW  = 4,
	CYAN    = 5,
	MAGENTA = 6,
	WHITE   = 7,
	GRAY    = 8,
	BLACK   = 9
};

/**
 * Initialize the colors to use with ncurses.
 */
void init_colors() {
	start_color();
	use_default_colors();
	init_pair(RED,     COLOR_RED,     -1);
	init_pair(GREEN,   COLOR_GREEN,   -1);
	init_pair(YELLOW,  COLOR_YELLOW,  -1);
	init_pair(GRAY,    COLOR_CYAN,    -1);
	init_pair(BLUE,    COLOR_BLUE,    -1);
	init_pair(CYAN,    COLOR_CYAN,    -1);
	init_pair(MAGENTA, COLOR_MAGENTA, -1);
	init_pair(WHITE,   COLOR_WHITE,   -1);
	init_pair(BLACK,   COLOR_BLACK,   -1);
}

/**
 * Used to define the game's difficulty setting.
 */
enum Difficulty {
	EASY, NORMAL, HARD
};

/**
 * Holds info on arguments passed from the CLI.
 */
struct CliArgs {
	Difficulty difficulty;
	bool sync_framerate;
	int esc_delay;
};

/**
 * Get the Difficulty from the CLI arguments.
 * @param argc The argument count.
 * @param argv The argument values.
 * @return The Difficulty, if one can be parsed from the arguments.
 */
CliArgs parse_cli_args(int argc, char** argv) {
	Difficulty difficulty = NORMAL;
	bool sync_framerate = false;
	int esc_delay = 100;

	bool display_help = false;
	bool unknown_arg = false;
	std::string unknown_arg_str = NULL;

	std::string esc_delay_prefix = "--esc_delay=";
	std::string e_prefix = "-e";

	int di = 0, si = 0, ei = 0;

	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];

		// first try difficulty
		if (arg.compare("help") == 0 ) {
			display_help = true;
			break;
		} else if (arg.compare("easy") == 0) {
			difficulty = EASY;
			di++;
		} else if (arg.compare("normal") == 0) {
			difficulty = NORMAL;
			di++;
		} else if (arg.compare("hard") == 0) {
			difficulty = HARD;
			di++;
		}
		// next try sync_framerate
		else if (arg.compare("--sync_framerate") == 0 || arg.compare("-s") == 0) {
			sync_framerate = true;
			si++;
		}
		// else  try esc_delay
		else if (arg.rfind(esc_delay_prefix, 0) == 0) {
			std::string substr = arg.substr(esc_delay_prefix.length());
			esc_delay = atoi(substr.c_str());
			ei++;
		} else if (arg.rfind(e_prefix, 0) == 0) {
			std::string substr = arg.substr(e_prefix.length());
			esc_delay = atoi(substr.c_str());
			ei++;
		}
		// otherwise it is unknown
		else {
			unknown_arg = true;
			break;
		}
	}
	bool duplicate_args = di > 1 || si > 1 || ei > 1;

	// print usage info if inputs are invalid
	if (display_help || unknown_arg || duplicate_args || argc > 4 || esc_delay < 100) {
		if (display_help) {
			std::cout << "Help info:" << std::endl;
		} else if (unknown_arg) {
			std::cout << "Unknown argument: " << unknown_arg_str << std::endl;
		} else if (duplicate_args) {
			std::cout << "Duplicate arguments." << std::endl;
		} else if (argc > 4) {
			std::cout << "Too many arguments." << std::endl;
		} else if (esc_delay < 100) {
			std::cout << "Too many arguments." << std::endl;
		}
		std::cout << "Usage: SnakeGame [ {easy|normal|hard} --esc_delay=milliseconds";
		std::cout << " --sync_framerate ]" << std::endl;
		std::cout << std::endl;
		std::cout << "Difficulty setting defaults to \"normal\"." << std::endl;
		std::cout << "\"--esc_delay\" (-eMilliseconds) defaults to 100, and must be >= 100.";
		std::cout << std::endl;
		std::cout << "\"--sync_framerate\" (-s) synchronizes horizontal and vertical frame rates.";
		std::cout << std::endl;
		std::cout << std::endl;
		std::cout << "Frame rates are by default faster horizontally to offset differences";
		std::cout << " in character height and width." << std::endl;
		std::cout << "If the game immediately quits as a result of pressing the arrow keys, try a";
		std::cout << " larger esc_delay." << std::endl;
		exit(0);
	}
	CliArgs cli_args = { difficulty, sync_framerate, esc_delay };
	return cli_args;
}

/**
 * Run the game.
 * @param argc The argument count.
 * @param argv The argument values.
 */

void snake_game(int argc, char** argv) {
	CliArgs cli_args = parse_cli_args(argc, argv);

	Difficulty difficulty = cli_args.difficulty;
	bool sync_framerate = cli_args.sync_framerate;
	int esc_delay = cli_args.esc_delay;

	long sleep_ms_horizontal;
	long sleep_ms_vertical;

	if (difficulty == EASY) {
		sleep_ms_horizontal = 66 * 1000;
		sleep_ms_vertical = 110 * 1000;
	} else if (difficulty == NORMAL) {
		sleep_ms_horizontal = 44 * 1000;
		sleep_ms_vertical = 77 * 1000;
	} else if (difficulty == HARD) {
		sleep_ms_horizontal = 22 * 1000;
		sleep_ms_vertical = 34 * 1000;
	} else {
		// unreachable code
		std::cout << "Unexpected error, difficulty = UNDEFINED." << std::endl;
		std::cout << "Exiting SnakeGame." << std::endl;
		exit(0);
	}

	if (sync_framerate) {
		sleep_ms_horizontal = sleep_ms_vertical;
	}

	initscr();
	cbreak();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);
	ESCDELAY = esc_delay;

	init_colors();

	int width = COLS;
	int height = LINES;

	// initialize RNG
	random_engine.seed(time(0));
	width_dist = std::uniform_int_distribution<int>(0, width - 1);
	height_dist = std::uniform_int_distribution<int>(0, height - 1);

	// initialize the Snake
	Snake* snake = new Snake();
	snake->direction = RIGHT;
	Point* start = new Point(width / 2, height / 2);
	snake->grow(start);

	// generate first treasure
	Point* treasure;
	bool generate_treasure = true;
	while (generate_treasure) {
		treasure = get_random_point();
		generate_treasure = snake->contains(treasure);
		if (generate_treasure) {
			treasure->Point::~Point();
		}
	}
	attron(COLOR_PAIR(YELLOW));
	mvaddch(treasure->y, treasure->x, '$');
	attron(COLOR_PAIR(GREEN));
	mvaddch(snake->head->y(), snake->head->x(), '@');
	refresh();

	// read user input on a separate thread.
	pthread_t thread_id;
	pthread_create(&thread_id, NULL, read_user_input, (void*) snake);

	// game loop
	while (true) {

		// sleep to control frame rate
		if (snake->direction == LEFT || snake->direction == RIGHT) {
			usleep(sleep_ms_horizontal);
		} else {
			usleep(sleep_ms_vertical);
		}
		if (snake->pause) {
			continue;
		}

		Point* next_move = snake->get_next_move();

		// check for game over conditions
		bool out_of_bounds = next_move->x < 0 || next_move->x >= width || next_move->y < 0
		        || next_move->y >= height;

		bool snake_collision = snake->contains(next_move);

		// display game over screen
		if (out_of_bounds || snake_collision) {
			clear();
			std::string game_over_text = "Game Over";
			int len = game_over_text.length();
			int gx = width / 2 - len / 2;
			int gy = height / 2;

			attron(COLOR_PAIR(RED));
			mvaddstr(gy, gx, game_over_text.c_str());

			int score = snake->segment_count;
			std::string score_key = "Score: ";
			std::string score_value = std::to_string(score);
			len = score_key.length() + score_value.length();
			gx = width / 2 - len / 2;
			gy += 1;

			attron(COLOR_PAIR(WHITE));
			move(gy, gx);
			addstr(score_key.c_str());

			attron(COLOR_PAIR(YELLOW));
			addstr(score_value.c_str());

			refresh();
			sleep(2);
			endwin();
			exit(0);
		}

		// check for collision with treasure
		if (next_move->equals(treasure)) {
			snake->grow(next_move);

			// check for victory condition
			int max_length = width * height;
			if (snake->segment_count == max_length) {
				clear();
				std::string victory_text = "Game Over";
				int len = victory_text.length();
				int gx = width / 2 - len / 2;
				int gy = height / 2;

				attron(COLOR_PAIR(GREEN));
				mvaddstr(gy, gx, victory_text.c_str());

				refresh();
				sleep(2);
				endwin();
				exit(0);
			}

			// generate new treasure
			generate_treasure = true;
			while (generate_treasure) {
				treasure = get_random_point();
				generate_treasure = snake->contains(treasure);
				if (generate_treasure) {
					treasure->Point::~Point();
				}
			}
			// draw new treasure
			attron(COLOR_PAIR(YELLOW));
			mvaddch(treasure->y, treasure->x, '$');
		} else {
			// un-draw last Segment of Snake
			mvaddch(snake->tail->y(), snake->tail->x(), ' ');
			snake->move(next_move);
		}
		// draw next Segment of Snake
		attron(COLOR_PAIR(GREEN));
		mvaddch(snake->head->y(), snake->head->x(), '@');
		refresh();
	}
	// unreachable, but included for posterity
	endwin();
}

/**
 * Test function to verify the accurateness of Snake and Segment's
 * linked-list functionality.
 */
void test_snake_segments() {
	Snake* snake = new Snake();
	Point* start_pos = new Point(1, 1);
	snake->grow(start_pos);

	std::cout << "Snake:" << std::endl;
	std::cout << "-------" << std::endl;
	std::cout << "direction: " << snake->direction << std::endl;
	std::cout << "head: " << snake->head->to_string() << std::endl;
	std::cout << "tail: " << snake->tail->to_string() << std::endl;
	std::cout << "segment_count: " << snake->segment_count << std::endl;

	for (Segment* s = snake->head; s != NULL; s = s->next) {
		std::cout << "    segment: " << snake->head->to_string() << std::endl;
	}
	std::cout << std::endl;

	snake->grow(new Point(1, 2));
	std::cout << "direction: " << snake->direction << std::endl;
	std::cout << "head: " << snake->head->to_string() << std::endl;
	std::cout << "tail: " << snake->tail->to_string() << std::endl;
	std::cout << "segment_count: " << snake->segment_count << std::endl;

	for (Segment* s = snake->head; s != NULL; s = s->next) {
		std::cout << "    segment: " << s->to_string() << std::endl;
	}
	std::cout << std::endl;

	snake->grow(new Point(1, 3));
	snake->grow(new Point(1, 4));
	std::cout << "direction: " << snake->direction << std::endl;
	std::cout << "head: " << snake->head->to_string() << std::endl;
	std::cout << "tail: " << snake->tail->to_string() << std::endl;
	std::cout << "segment_count: " << snake->segment_count << std::endl;

	for (Segment* s = snake->head; s != NULL; s = s->next) {
		std::cout << "    segment: " << s->to_string() << std::endl;
	}
	std::cout << std::endl;

	snake->move(new Point(2, 4));
	std::cout << "direction: " << snake->direction << std::endl;
	std::cout << "head: " << snake->head->to_string() << std::endl;
	std::cout << "tail: " << snake->tail->to_string() << std::endl;
	std::cout << "segment_count: " << snake->segment_count << std::endl;

	for (Segment* s = snake->head; s != NULL; s = s->next) {
		std::cout << "    segment: " << s->to_string() << std::endl;
	}
	std::cout << std::endl;
}

/**
 * Run the program.
 * @param argc The argument count.
 * @param argv The argument values.
 * @return zero (never reached, but included for posterity.)
 */
int main(int argc, char** argv) {
//	test_snake_segments();
	snake_game(argc, argv);
	return 0;
}