/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <exception>

//#include <boost/program_options.hpp>

#include "rat_trap_parts.hpp"

//namespace po = boost::program_options;

int main() try {
	/*po::options_description cmd_desc("Command-line options");
	cmd_desc.add_options()
		("help,h", "this helpful message")
		("conf,c", po::value<std::string>()->default_value("./"), "configuration file path")
		;
	po::options_description gen_desc("Generic options");
	gen_desc.add_options()
		("hunspell", po::value<std::string>()->default_value(), "hunspell dictionary path")
		;*/
	rat_trap_parts r;
	r.go();
	return 0;
} catch(std::exception &e) {
	fprintf(stderr, "%s\n", e.what());
	return 1;
};
