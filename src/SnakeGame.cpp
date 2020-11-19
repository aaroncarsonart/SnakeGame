//============================================================================
// Name        : SnakeGame.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : The classic SnakeGame, implemented in C++ for the terminal.
//============================================================================

#include <iostream>
#include <string>
#include <cstring>
#include <random>
#include <regex>

#include <ncurses.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#include "Point.h"
#include "Keys.h"
#include "Snake.h"

#define ENUM_CAST(e) static_cast<typename std::underlying_type<Direction>::type>(e)

/**
 * Read the user input.  Intended to be run on a separate thread
 * via passing as an argument to {@code pthread_create}.
 * @param vargp The Snake to be updated by the user input.
 */
void* read_user_input(void* vargp) {
	Snake* snake = (Snake*) vargp;
	while (!snake->game_over) {
		int input = getch();
		switch (input) {
		case A_KEY_A:
		case A_KEY_H:
		case A_KEY_LEFT:
		case KEY_LEFT:
			if (snake->direction != Direction::LEFT && snake->direction != Direction::RIGHT) {
				snake->direction = Direction::LEFT;
				snake->pause = false;
			}
			break;
		case A_KEY_S:
		case A_KEY_J:
		case A_KEY_DOWN:
		case KEY_DOWN:
			if (snake->direction != Direction::DOWN && snake->direction != Direction::UP) {
				snake->direction = Direction::DOWN;
				snake->pause = false;
			}
			break;
		case A_KEY_W:
		case A_KEY_K:
		case A_KEY_UP:
		case KEY_UP:
			if (snake->direction != Direction::UP && snake->direction != Direction::DOWN) {
				snake->direction = Direction::UP;
				snake->pause = false;
			}
			break;
		case A_KEY_D:
		case A_KEY_L:
		case A_KEY_RIGHT:
		case KEY_RIGHT:
			if (snake->direction != Direction::RIGHT && snake->direction != Direction::LEFT) {
				snake->direction = Direction::RIGHT;
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
//		case A_KEY_C:
//			snake->segment_count = COLS * LINES - 1;
//			break;
		}
	}
	return nullptr;
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
	init_pair(RED,     COLOR_RED,     COLOR_BLACK);
	init_pair(GREEN,   COLOR_GREEN,   COLOR_BLACK);
	init_pair(YELLOW,  COLOR_YELLOW,  COLOR_BLACK);
	init_pair(GRAY,    COLOR_CYAN,    COLOR_BLACK);
	init_pair(BLUE,    COLOR_BLUE,    COLOR_BLACK);
	init_pair(CYAN,    COLOR_CYAN,    COLOR_BLACK);
	init_pair(MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(WHITE,   COLOR_WHITE,   COLOR_BLACK);
	init_pair(BLACK,   COLOR_BLACK,   COLOR_BLACK);
}

/**
 * Used to define the game's difficulty setting.
 */
enum class Difficulty {
	EASY, NORMAL, HARD
};

/**
 * Holds info on arguments passed from the CLI.
 */
struct CliArgs {
	Difficulty difficulty;
	bool sync_frame_rate;
	bool enable_colors;
	int esc_delay;
};

/**
 * Get the Difficulty from the CLI arguments.
 * @param argc The argument count.
 * @param argv The argument values.
 * @return The Difficulty, if one can be parsed from the arguments.
 */
CliArgs parse_cli_args(int argc, char** argv) {
	Difficulty difficulty = Difficulty::NORMAL;
	bool sync_frame_rate = false;
	int esc_delay = 100;

	bool display_help = false;
	bool enable_colors = true;

	std::string unknown_arg;

	std::string esc_delay_prefix = "--esc_delay=";
	std::string e_prefix = "-e";

	int hi = 0, di = 0, si = 0, ci = 0, ei = 0;

	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];

		// first try difficulty
		if (arg.compare("help") == 0 || arg.compare("--help") == 0 ) {
			display_help = true;
			hi++;
		} else if (arg.compare("easy") == 0) {
			difficulty = Difficulty::EASY;
			di++;
		} else if (arg.compare("normal") == 0) {
			difficulty = Difficulty::NORMAL;
			di++;
		} else if (arg.compare("hard") == 0) {
			difficulty = Difficulty::HARD;
			di++;
		}
		// next try sync_frame_rate
		else if (arg.compare("--sync_frame_rate") == 0) {
			sync_frame_rate = true;
			si++;
		}
		// next try disable_color
		else if (arg.compare("--disable_colors") == 0) {
			enable_colors = false;
			ci++;
		}
		// next try esc_delay
		else if (arg.rfind(esc_delay_prefix, 0) == 0) {
			std::string substr = arg.substr(esc_delay_prefix.length());
			esc_delay = atoi(substr.c_str());
			ei++;
		} else if (arg.rfind(e_prefix, 0) == 0) {
			std::string substr = arg.substr(e_prefix.length());
			esc_delay = atoi(substr.c_str());
			ei++;
		}
		// else try optional flags
		else if (std::regex_match(arg, std::regex("-[dhs]{1,2}"))) {
			for (unsigned int i = 1; i < arg.length(); i++) {
				char c = arg[i];
				switch (c) {
				case 'd':
					enable_colors = false;
					ci++;
					break;
				case 'h':
					display_help = true;
					hi++;
					break;
				case 's':
					sync_frame_rate = true;
					si++;
					break;
				}
			}

		}
		// otherwise it is unknown
		else {
			unknown_arg = arg;
			break;
		}
	}
	bool duplicate_args = hi > 1 || di > 1 || si > 1 || ci > 1 || ei > 1;

	// print usage info if inputs are invalid
	if (display_help || unknown_arg.length() > 0 || duplicate_args || esc_delay < 100) {
		if (display_help) {
			std::cout << "Help info:" << std::endl;
		} else if (unknown_arg.length() > 0) {
			std::cout << "Unknown argument: " << unknown_arg << std::endl;
		} else if (duplicate_args) {
			std::cout << "Duplicate arguments." << std::endl;
		} else if (argc > 4) {
			std::cout << "Too many arguments." << std::endl;
		} else if (esc_delay < 100) {
			std::cout << "esc_delay of " << esc_delay << " is too small." << std::endl;
		}
		std::cout << std::endl;
		std::cout << "Usage: SnakeGame [ {easy|normal|hard} -dhs -eMilliseconds ]" << std::endl;
		std::cout << std::endl;
		std::cout << "Difficulty setting defaults to \"normal\"." << std::endl;
		std::cout << std::endl;
		std::cout << "Options:" << std::endl;
		std::cout << "\"--disable_colors\" (-d) disables color output." << std::endl;
		std::cout << "\"--esc_delay=milliseconds\" (-eMilliseconds) defaults to 100, and must be >="
		          << "100." << std::endl;
		std::cout << "\"--help\" (-h) displays this help info." << std::endl;
		std::cout << "\"--sync_frame_rate\" (-s) synchronizes horizontal and vertical frame rates."
		          << std::endl;
		std::cout << std::endl;
		std::cout << std::endl;
		std::cout << "Frame rates are by default faster horizontally to offset differences in "
		          << "character height and width." << std::endl;
		std::cout << "If the game immediately quits as a result of pressing the arrow keys, try a "
		          << "larger esc_delay." << std::endl;
		exit(0);
	}
	CliArgs cli_args = { difficulty, sync_frame_rate, enable_colors, esc_delay };
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
	bool sync_frame_rate = cli_args.sync_frame_rate;
	bool enable_colors = cli_args.enable_colors;
	int esc_delay = cli_args.esc_delay;

	long sleep_ms_horizontal;
	long sleep_ms_vertical;

	if (difficulty == Difficulty::EASY) {
		sleep_ms_horizontal = 66 * 1000;
		sleep_ms_vertical = 110 * 1000;
	} else if (difficulty == Difficulty::NORMAL) {
		sleep_ms_horizontal = 44 * 1000;
		sleep_ms_vertical = 77 * 1000;
	} else if (difficulty == Difficulty::HARD) {
		sleep_ms_horizontal = 22 * 1000;
		sleep_ms_vertical = 34 * 1000;
	} else {
		// unreachable code
		std::cout << "Unexpected error, difficulty = UNDEFINED." << std::endl;
		std::cout << "Exiting SnakeGame." << std::endl;
		exit(0);
	}

	if (sync_frame_rate) {
		sleep_ms_horizontal = sleep_ms_vertical;
	}

	initscr();
	cbreak();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);
	ESCDELAY = esc_delay;

	if (enable_colors) {
		init_colors();
	}

	int width = COLS;
	int height = LINES;

	// init the background color
	attron(COLOR_PAIR(BLACK));
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			mvaddch(y, x, ' ');
		}
	}

	// initialize RNG
	random_engine.seed(time(0));
	width_dist = std::uniform_int_distribution<int>(0, width - 1);
	height_dist = std::uniform_int_distribution<int>(0, height - 1);

	// initialize the Snake
	Snake* snake = new Snake();
	snake->direction = Direction::RIGHT;
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
	pthread_t read_input_thread;
	pthread_create(&read_input_thread, nullptr, read_user_input, (void*) snake);

	// game loop
	while (true) {

		// sleep to control frame rate
		if (snake->direction == Direction::LEFT || snake->direction == Direction::RIGHT) {
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
			// clear the screen
			attron(COLOR_PAIR(BLACK));
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					mvaddch(y, x, ' ');
				}
			}
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

			snake->game_over = true;
			pthread_join(read_input_thread, nullptr);

			endwin();
			exit(0);
		}

		// check for collision with treasure
		if (next_move->equals(treasure)) {
			snake->grow(next_move);

			// check for victory condition
			int max_length = width * height;
			if (snake->segment_count == max_length) {
				// clear the screen
				attron(COLOR_PAIR(BLACK));
				for (int y = 0; y < height; y++) {
					for (int x = 0; x < width; x++) {
						mvaddch(y, x, ' ');
					}
				}
				std::string victory_text = "Congratulations,";
				int len = victory_text.length();
				int gx = width / 2 - len / 2;
				int gy = height / 2 - 1;

				attron(COLOR_PAIR(WHITE));
				mvaddstr(gy, gx, victory_text.c_str());

				victory_text = "you win!";
				len = victory_text.length();
				gx = width / 2 - len / 2;
				gy += 1;

				mvaddstr(gy, gx, victory_text.c_str());

				int score = snake->segment_count;
				std::string score_key = "Maximum Score: ";
				std::string score_value = std::to_string(score);
				len = score_key.length() + score_value.length();
				gx = width / 2 - len / 2;
				gy += 1;

				move(gy, gx);
				addstr(score_key.c_str());

				attron(COLOR_PAIR(GREEN));
				addstr(score_value.c_str());

				refresh();

				snake->game_over = true;
				pthread_join(read_input_thread, nullptr);

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
 * Print the fields of the given Snake struct.
 * @param snake The Snake to print the fields of.
 */
void print_fields(Snake* snake) {
	std::cout << "Snake:" << std::endl;
	std::cout << "-------" << std::endl;
	std::cout << "direction: " << ENUM_CAST(snake->direction) << std::endl;
	std::cout << "head: " << snake->head->to_string() << std::endl;
	std::cout << "tail: " << snake->tail->to_string() << std::endl;
	std::cout << "segment_count: " << snake->segment_count << std::endl;

	for (Segment* s = snake->head; s != nullptr; s = s->next) {
		std::cout << "    segment: " << s->to_string() << std::endl;
	}
	std::cout << std::endl;
}

/**
 * Test function to verify the accurateness of Snake and Segment's
 * linked-list functionality.
 */
void test_snake_segments() {
	Snake* snake = new Snake();
	Point* start_pos = new Point(1, 1);
	snake->grow(start_pos);
	print_fields(snake);

	snake->grow(new Point(1, 2));
	print_fields(snake);

	snake->grow(new Point(1, 3));
	snake->grow(new Point(1, 4));
	print_fields(snake);

	snake->move(new Point(2, 4));
	print_fields(snake);
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
