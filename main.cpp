#include <cassert>
#include <exception>

#include <algorithm>
#include <random>

#include <array>
#include <set>
#include <string>
#include <vector>

#include <ncurses.h>
#include <hunspell/hunspell.hxx> // for stem
#include <wn.h> // for in_wn and morphword

//TODO: put this in a config like a decent human being
#define HUNSPELL_AFF "/opt/local/share/hunspell/en_US.aff"
#define HUNSPELL_DIC "/opt/local/share/hunspell/en_US.dic"

#define MAX_ROWS 24
#define MAX_COLS 80
#define PROMPT_ROW 23
#define SCORE_ROW 0
#define PRIOR_START 2
#define PRIOR_END 16
#define CURRENT_START 18
#define CURRENT_END 21
#define ERROR_ROW 22

static const int BLANK_ROWS[2] = {1, 17};

#define SCORE_STR "Score:"
#define FINAL_SCORE_STR "Your final score is "
#define PROMPT_STR ">"

static Hunspell spell(HUNSPELL_AFF, HUNSPELL_DIC);

struct word {
	std::string literal;
	std::string sorted;

	word(std::string w) : literal(w), sorted(w) {
		std::sort(sorted.begin(), sorted.end());
	}

	bool operator< (word const& other) const {
		return literal < other.literal;
	}

	bool is_one_less_than(std::vector<std::string const>& other) const {
		std::string o;
		for (auto const& i : other) {
			o += i;
		}

		// is length mismatched?
		if (o.size() - sorted.size() != 1) {
			return false;
		}

		// ensure we differ by only one letter
		std::sort(o.begin(), o.end());
		int i, j;
		i = j = 0;
		while (i < sorted.size() && j < o.size()) {
			if (sorted[i] == o[j]) {
				i++;
				j++;
				continue;
			}
			if (j - i > 1) return false;
			j++;
		}
		return true;
	}
};

template<unsigned long size> void paginate(std::set<word const> const& from,
		std::vector<std::array<std::string, size> >& to) {
	to.clear();
	to.emplace_back();
	std::string row;
	int row_index = 0;
	for (auto const& str : from) {
		if (row.size() + str.literal.size() < MAX_COLS) {
			row += str.literal + " ";
			continue;
		}
		if (row_index == size) {
			to.emplace_back();
			row_index = 0;
		}
		to.back()[row_index++] = row;
		row.clear();
	}
	if (row.size() > 0) {
		if (row_index == size) {
			to.emplace_back();
			row_index = 0;
		}
		to.back()[row_index++] = row;
	}
}

class rat_trap_parts {
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

	std::set<std::string const> stems_from_str(std::string const& str) {
		std::set<std::string const> stems;
		char literal_arr[128];

		if (str.size() >= sizeof(literal_arr)) {
			return stems;
		}

		std::string literal = str;

		if (!lowercase_and_validate(literal)) {
			return stems;
		}

		bool should_hunspell = false;
		strcpy(literal_arr, literal.c_str());
		// morph the str to base form first
		for (int i = NOUN; i <= ADV; i++) {
			char* buf = morphword(literal_arr, i);
			if (buf == nullptr) {
				if (in_wn(literal_arr, i)) {
					should_hunspell = true;
				}
				continue;
			}
			stems.emplace(buf);
		}
		// then try stemming it
		if (should_hunspell) {
			char** stems_arr;
			int stems_count = spell.stem(&stems_arr, literal_arr);
			for(int i = 0; i < stems_count; i++) {
				stems.emplace(stems_arr[i]);
				i++;
			}
			if (stems_count > 0) {
				spell.free_list(&stems_arr, stems_count);
			}
		}

		return stems;
	};

	void adjust_screen_dimensions() {
		int row, col;
		getmaxyx(stdscr, row, col);
		if (ERR == wresize(stdscr, row < MAX_ROWS ? MAX_ROWS : row,
					col < MAX_COLS ? MAX_COLS : col)) {
			throw std::runtime_error(
					"Failed to resize ncurses window to 80x24 or greater.");
		}
	};

	bool lowercase_and_validate(std::string &str) {
		for (auto &c : str) {
			c = tolower(c);
			if (c < 'a' || c > 'z') {
				return false;
			}
		}
		return true;
	}

	void help() {
		//TODO
	}

	void setup() {
		while(current.size() == 0) {
			clear();
			attron(A_REVERSE);
			mvprintw(21, 0, "Enter a 3-letter word to start with.");
			mvprintw(22, 0, "'r' or 'random' for random start.");
			mvprintw(PROMPT_ROW, 0, PROMPT_STR);
			attroff(A_REVERSE);
			refresh();
			mvgetnstr(PROMPT_ROW, 2, input_arr, sizeof(input_arr));
			std::string str(input_arr);
			if (lowercase_and_validate(str)) {
				if (str.size() == 3 && spell.spell(str.c_str())) {
					current.insert(str);
					std::set<std::string const> stems = stems_from_str(str);
					used_stems.insert(stems.begin(), stems.end());
					return;
				} else if (str == "r" || str == "random") {
					FILE* f = fopen("valid_words.txt", "r");
					if (f == nullptr) {
						throw std::runtime_error("Couldn't read valid words.");
					}
					std::vector<std::string const> choices;
					char data[10240];
					int read = fread(data, 4, sizeof(data)/4, f);
					assert(read > 0);
					fclose(f);
					if (ferror(f) != 0) {
						throw std::runtime_error("Couldn't read valid words.");
					}
					for (char* start = data; start < data + read*4 - 3; start += 4) {
						start[3] = '\0';
						choices.emplace_back(start);
					}
					std::string choice = choices[std::random_device()()%choices.size()];
					current.insert(choice);
					std::set<std::string const> stems = stems_from_str(choice);
					used_stems.insert(stems.begin(), stems.end());
					return;
				}
			}
		}
	}

	void play() {
		setup();
		clear();

		paginate(prior, prior_strings);
		paginate(current, current_strings);
		fprintf(stderr, "%s", "ok here");

		char line_buffer[MAX_COLS + 1];
		for (int i = 0; i < MAX_COLS; i++) line_buffer[i] = ' ';
		line_buffer[MAX_COLS] = '\0';
		attron(A_REVERSE);
		mvprintw(ERROR_ROW, 0, line_buffer);
		attroff(A_REVERSE);
		while (true) {
			for (int i = 0; i < MAX_COLS; i++) line_buffer[i] = ' ';
			line_buffer[MAX_COLS] = '\0';
			attron(A_REVERSE);
			mvprintw(SCORE_ROW, 0, SCORE_STR);
			mvprintw(PROMPT_ROW, 0, PROMPT_STR);
			for (auto const i : BLANK_ROWS) mvprintw(i, 0, line_buffer);
			attroff(A_REVERSE);
			snprintf(line_buffer, MAX_COLS, " %lu", score);
			mvprintw(SCORE_ROW, sizeof(SCORE_STR), line_buffer);
			if (prior_strings.size() > 0) {
				for (int i = PRIOR_START; i <= PRIOR_END; i++) {
					mvprintw(i, 0,
							prior_strings[prior_index].data()[i - PRIOR_START].c_str());
				}
			}
			assert(current_strings.size() > 0);
			for (int i = CURRENT_START; i <= CURRENT_END; i++) {
				mvprintw(i, 0,
						current_strings[current_index].data()[i - CURRENT_START].c_str());
			}

			refresh();
			mvgetnstr(23, 1, input_arr, sizeof(input_arr));
			clear();
			for (int i = 0; i < MAX_COLS; i++) line_buffer[i] = ' ';
			line_buffer[MAX_COLS] = '\0';
			attron(A_REVERSE);
			mvprintw(ERROR_ROW, 0, line_buffer);
			attroff(A_REVERSE);
			std::string input(input_arr);
			for (auto& c : input) {
				c = tolower(c);
			}
			if (input == ",") {
				prior_index = std::max(static_cast<unsigned int>(0), prior_index - 1);
				continue;
			} else if (input == ".") {
				prior_index = std::min(prior_strings.size(),
						static_cast<unsigned long>(prior_index + 1));
				continue;
			} else if (input == "<") {
				current_index = std::max(static_cast<unsigned int>(0),
						current_index - 1);
				continue;
			} else if (input == ">") {
				current_index = std::min(current_strings.size(),
						static_cast<unsigned long>(current_index + 1));
				continue;
			} else if (input == "q") {
				for (auto const& c : current) {
					score += c.literal.size() - 3;
				}
				snprintf(line_buffer, MAX_COLS, "Your final score is %lu", score);
				mvprintw(SCORE_ROW, 0, line_buffer);
				mvprintw(PROMPT_ROW, 0, "Press any key to continue...");
				refresh();
				noecho();
				getch();
				echo();
				return;
			} else if (input == "?" || input == "h") {
				help();
				clear();
				continue;
			}

			char* start = input_arr;
			char* token;

			// is the first word in our current set?
			token = strsep(&start, " ");
			std::string chosen(token);
			if (current.count(chosen) == 0) {
				attron(A_REVERSE);
				snprintf(line_buffer, MAX_COLS, "'%s' is not a current word.",
						chosen.c_str());
				mvprintw(ERROR_ROW, 0, line_buffer);
				attroff(A_REVERSE);
				continue;
			}

			// make sure the candidates are are lowercase alpha and at least 3 chars
			// long
			std::vector<std::string const> candidates;
			bool entry_invalid = false;
			if (start == nullptr) {
				attron(A_REVERSE);
				snprintf(line_buffer, MAX_COLS, "Need at least one word...");
				mvprintw(ERROR_ROW, 0, line_buffer);
				attroff(A_REVERSE);
				continue;
			}

			while(start != nullptr) {
				std::string str(strsep(&start, " "));
				if (!lowercase_and_validate(str) || str.size() < 3) {
					attron(A_REVERSE);
					snprintf(line_buffer, MAX_COLS, "'%s' is not alpha/too short",
							str.c_str());
					mvprintw(ERROR_ROW, 0, line_buffer);
					attroff(A_REVERSE);
					entry_invalid = true;
					break;
				}
				candidates.push_back(str);
			}
			if (entry_invalid) continue;

			int score_this_round = 0;
			std::set<std::string const> stems_this_round;
			for (auto const& candidate : candidates) {
				std::set<std::string const> stems = stems_from_str(candidate);
				// is this even a real word?
				if (stems.size() == 0) {
					attron(A_REVERSE);
					snprintf(line_buffer, MAX_COLS, "'%s' isn't a valid word",
							candidate.c_str());
					mvprintw(ERROR_ROW, 0, line_buffer);
					attroff(A_REVERSE);
					entry_invalid = true;
					break;
				}
				// is at least one stem of this word used?
				bool was_scored = false;
				for (auto const& stem : stems) {
					if (used_stems.count(stem) == 0 && stems_this_round.count(stem) == 0) {
						stems_this_round.insert(stem);
						if (!was_scored) {
							score_this_round += candidate.size() - 3;
							was_scored = true;
						}
					} else {
						attron(A_REVERSE);
						snprintf(line_buffer, MAX_COLS, "'%s' already used previously",
								candidate.c_str());
						mvprintw(ERROR_ROW, 0, line_buffer);
						attroff(A_REVERSE);
						entry_invalid = true;
						break;
					}
				}
			}
			if (entry_invalid) continue;
			score += score_this_round;
			used_stems.insert(stems_this_round.begin(), stems_this_round.end());
			current.erase(chosen);
			prior.insert(chosen);
			current.insert(candidates.begin(), candidates.end());
			paginate(prior, prior_strings);
			paginate(current, current_strings);
		}
	};

	public:
	rat_trap_parts() : prior_index(0), current_index(0), score(0) {
		if (wninit() != 0) {
			throw std::runtime_error("Failed to initialize WordNet.");
		}
		if (initscr() == nullptr) {
			throw std::runtime_error("Failed to initialize ncurses.");
		}
	};

	~rat_trap_parts() {
		endwin();
	};

	void go() {
		adjust_screen_dimensions();
		echo();

		play();
	};
};

int main() try {
	rat_trap_parts r;
	r.go();
	return 0;
} catch(std::exception &e) {
	fprintf(stderr, "%s\n", e.what());
	return 1;
};
