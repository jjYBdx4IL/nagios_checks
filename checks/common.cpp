
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <boost/date_time.hpp>

#include "common.h"

std::time_t tnow = std::time(nullptr);

std::string ssystem(std::string command)
{
	char tmpname[L_tmpnam];
	std::tmpnam(tmpname);
	std::string scommand = command;
	std::string cmd = scommand + " >> " + tmpname;
	std::system(cmd.c_str());
	std::ifstream file(tmpname, std::ios::in | std::ios::binary);
	std::string result;
	if (file) {
		while (!file.eof()) result.push_back(file.get())
			;
		file.close();
	}
	remove(tmpname);
	return result;
}

void err(int sc, std::string msg)
{
	std::cout << sc << ":" << msg << std::endl;
	exit(sc);
}

std::time_t parse_to_time_t(std::string s)
{
	// older GNU system libs and Boost do not allow for single digit day of month
	// -> replace preceding space with "0"
	if (s.length() >= 5 && s.substr(3,2) == "  ") {
		s = s.substr(0, 4) + "0" + s.substr(5);
#ifdef _DEBUG
		std::cout << "added 0:" << s << std::endl;
#endif
	}

	namespace bt = boost::posix_time;

	bt::ptime pt;
	std::istringstream is(s);
	is.imbue(std::locale(std::locale::classic(), new bt::time_input_facet("%b %d %H:%M:%S %Y")));
	is >> pt;
	_assert(pt != bt::ptime());

	bt::ptime timet_start(boost::gregorian::date(1970, 1, 1));
	bt::time_duration diff = pt - timet_start;
	return diff.ticks() / bt::time_duration::rep_type::ticks_per_second;
}

long long age(std::string s)
{
	std::time_t tt = parse_to_time_t(s);
	long long secs = std::difftime(tnow, tt);
#ifdef _DEBUG
	std::cout << "age = " << secs << std::endl;
#endif
	_assert(secs > -60);
	if (secs < 0) {
		secs = 0;
	}
	return secs;
}

