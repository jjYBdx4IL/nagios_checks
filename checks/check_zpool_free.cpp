#include "check_zpool_free.h"

namespace fs = std::filesystem;

class Pool {
public:
	std::string name, size, alloc, free, ckpoint, expandsz, frag, cap, dedup, health, altroot;
};

class PoolArg {
public:
	std::string name;
	double freePctW = 10, freePctC = 5;
};

std::vector<PoolArg> poolArgs;

PoolArg getPoolArg(std::string name) {
	for (PoolArg& pa : poolArgs) {
		if (pa.name == name) {
			return pa;
		}
	}
	return PoolArg();
}

fs::path zpoolExe = ZPOOL_EXE;

int main(int argc, char** argv)
{
	//
	// parse command line
	//

	std::regex cmdRegex("^--(\\S+?)(?:=(.*))?$");
	std::regex freeArgRegex("^(\\S*?),(\\d+)%(?:,(\\d+)%)?$");
	std::smatch sm;
	for (int i = 1; i < argc; i++) {
		std::string arg(argv[i]);

		if (!std::regex_match(arg, sm, cmdRegex)) {
			err(3, "invalid command line argument: " + arg);
		}

		std::string cmd = sm[1].str();
		std::string cmdArg = sm[2].str();

		// for testing only:
		if (cmd == "exe") {
			zpoolExe = fs::path(cmdArg).make_preferred();
			_assert(fs::exists(zpoolExe));
			_assert(fs::is_symlink(zpoolExe) || fs::is_regular_file(zpoolExe));
			continue;
		}
		else if (cmd == "tnow") {
			//ignore
		}
		// "--free=rpool,10%" or "--free=rpool,10%,20%".
		// empty pool name for default setting.
		else if (cmd == "free" && std::regex_match(cmdArg, sm, freeArgRegex)) {
			PoolArg poolArg;
			poolArg.name = sm[1];
			poolArg.freePctC = poolArg.freePctW = std::stol(sm[2]);
			if (sm[3].matched) {
				poolArg.freePctW = std::stol(sm[3]);
			}
			if (poolArg.freePctC > poolArg.freePctW) {
				std::swap(poolArg.freePctC, poolArg.freePctW);
			}
			if (getPoolArg(poolArg.name).name != "") {
				err(3, "pool defined twice: " + poolArg.name);
			}
			poolArgs.push_back(poolArg);
		}
		else if (cmd == "help") {
			std::cout
				<< "usage: check_zpool_free [--free=<POOL>,10%[,20%]] [...]" << std::endl
				<< "By default, warning/critical free capacity values are 10% and 5% for each device" << std::endl
				<< "found in 'zpool list' output. The lower of both values is always the critical one." << std::endl
				<< "You can set default values using the empty pool name." << std::endl;
			exit(5);
		}
		else {
			err(3, "invalid command: " + arg);
		}
	}

	//
	// parse "zpool list -H" output
	//

	std::string s = ssystem(zpoolExe.string() + " list -H");
	std::istringstream iss(s);
	std::string line;
	//sysbkp@nas:~$ sudo zpool list -H
	//bpool   480M    97.7M   382M    -       -       4%      20%     1.00x   ONLINE  -
	//rpool   3.62T   1.04T   2.58T   -       -       0%      28%     1.00x   ONLINE  -
	std::regex listRegex("^(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)%\\s+(\\S+)%\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)[\\r\\n]*$");
	std::smatch m;
	std::vector<Pool> pools;
	int n = 0;
	while (std::getline(iss, line))
	{
		if (std::regex_match(line, m, listRegex)) {
			Pool p;
			p.name = m[1];
			p.size = m[2];
			p.alloc = m[3];
			p.free = m[4];
			p.ckpoint = m[5];
			p.expandsz = m[6];
			p.frag = m[7];
			p.cap = m[8];
			p.dedup = m[9];
			p.health = m[10];
			p.altroot = m[11];
			pools.push_back(p);
		}
	}

	//
	// evaluate parsed output
	//

	if (pools.empty()) {
		err(2, "no pools found");
	}

	// have we found every pool given on the command line?
	for (PoolArg& poolArg : poolArgs) {
		if (poolArg.name == "")
			continue;
		bool found = false;
		for (Pool& pool : pools) {
			if (pool.name == poolArg.name) {
				found = true;
				break;
			}
		}
		if (!found)
			err(2, "pool not found in output - " + poolArg.name);
	}

	// check free space in all parsed pools
	int sc = 0;
	std::vector<std::string> msgs;

	for (Pool& p : pools) {
		long cap = std::stol(p.cap);
		long freePct = 100 - cap;

		// appened info to message output
		std::stringstream msg;
		msg << p.name << "=" << freePct << "%(alloc=" << p.alloc << "/" << p.size << ")";
		msgs.push_back(msg.str());

		// check free space
		PoolArg pa = getPoolArg(p.name);
		if (freePct < pa.freePctC) {
			sc = std::max(2, sc);
		}
		else if (freePct < pa.freePctW) {
			sc = std::max(1, sc);
		}
	}

	err(sc, boost::algorithm::join(msgs, " / "));
}
