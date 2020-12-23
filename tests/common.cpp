#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>

using namespace std;

namespace fs = std::filesystem;

void writeFile(fs::path outFile, std::string content)
{
	ofstream myfile;
	myfile.open(outFile.make_preferred(), std::ofstream::trunc);
	myfile << content;
	myfile.close();
}
