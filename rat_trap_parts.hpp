#pragma once
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <set>
#include <string>
#include <vector>

#include <hunspell/hunspell.hxx> // for stem

#include "ncurses_wrappers.hpp"

struct word {
	std::string literal;
	std::string sorted;

	word(std::string const& w);
	bool operator< (word const& other) const;
	bool is_one_less_than(std::vector<std::string const>& other) const;
};

class rat_trap_parts {
	Hunspell spell;

	char input_arr[128];

	std::set<const word> current;
	std::set<const word> prior;
	std::vector<std::array<std::string, PRIOR_END - PRIOR_START + 1> >
		prior_strings;
	std::vector<std::array<std::string, CURRENT_END - CURRENT_START + 1> >
		current_strings;
	unsigned int prior_index;
	unsigned int current_index;
	std::set<const std::string> used_stems;
	unsigned long score;

	std::vector<std::string const> readme_lines;

	std::set<std::string const> stems_from_str(std::string const& str);
	void adjust_screen_dimensions();
	void help();
	void setup();
	void play();

	public:
	rat_trap_parts();
	~rat_trap_parts();
	void go();
};
