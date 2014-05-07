/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <string>

#include <ncurses.h>

#include "ncurses_wrappers.hpp"

const static std::string blank_row(MAX_COLS, ' ');

void rmvprintw(int row, int col, char const* str) {
	attron(A_REVERSE);
	mvprintw(row, col, str);
	attroff(A_REVERSE);
}

void print_err(char const* fmt, ...) {
	static char line_buffer[MAX_COLS + 1];

	va_list args;
	va_start(args, fmt);

	int count = vsnprintf(line_buffer, MAX_COLS, fmt, args);
	memset(line_buffer + count, ' ', MAX_COLS - count);
	line_buffer[MAX_COLS] = '\0';
	rmvprintw(ERROR_ROW, 0, line_buffer);

	va_end(args);
}

void print_blank(int row) {
	rmvprintw(row, 0, blank_row.c_str());
}
