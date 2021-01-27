#pragma once

#include <string>
#include <filesystem>
#include "common.h"

#define TNOW " --tnow=1607845120 "

int runit(std::string binary, std::string statusFn, std::string cmdArg, std::string tnow = TNOW);
int runitStr(std::string binary, std::string content, std::string cmdArg);
