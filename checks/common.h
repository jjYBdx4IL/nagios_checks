#pragma once

#include <filesystem>

#define ZPOOL_EXE "zpool"

#define _assert(cond) {if(!(cond)) {std::cerr << "3:assertion error at " << __FILE__ << "@" << __LINE__ << std::endl; exit(3);}}

std::string ssystem(std::string command);
void err(int sc, std::string msg);

extern std::time_t tnow;
long long age(std::string s);
