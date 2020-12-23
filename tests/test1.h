#pragma once

#include <string>
#include <filesystem>
#include "common.h"

int runit(std::string binary, std::string statusFn, std::string cmdArg);
int runitStr(std::string binary, std::string content, std::string cmdArg);
