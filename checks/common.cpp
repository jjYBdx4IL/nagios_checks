
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "common.h"

std::time_t tnow = std::time(nullptr);

std::string ssystem(std::string command) {
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

void err(int sc, std::string msg) {
	std::cout << sc << ":" << msg << std::endl;
	exit(sc);
}

long long age(std::string s) {
	std::tm t = {};
	std::istringstream ss(s);
	ss >> std::get_time(&t, "%b %d %H:%M:%S %Y");
	_assert(!ss.fail());
	std::time_t tt = std::mktime(&t);
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

