#pragma once

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

void writeFile(fs::path outFile, std::string content);
