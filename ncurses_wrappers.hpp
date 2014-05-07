#pragma once
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <ncurses.h>

#define MAX_ROWS 24
#define MAX_COLS 80
#define PROMPT_ROW 23
#define SCORE_ROW 0
#define PRIOR_START 2
#define PRIOR_END 16
#define CURRENT_START 18
#define CURRENT_END 21
#define ERROR_ROW 22

void rmvprintw(int row, int col, char const* str);
void print_err(char const* fmt, ...);
void print_blank(int row=ERROR_ROW);
