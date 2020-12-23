#include "check_zpool.h"

#define MAX_SCRUBBED_AGE_SECS (10L * 86400)
#define MAX_SCRUBBING_SINCE_SECS (3L * 86400)
#define MAX_RESILVERING_SINCE_SECS (3L * 86400)

class Pool {
public:
	std::string name;
	std::string state;
	std::string errors;
	std::string scrubbedOn;
	std::string scrubbedRepaired;
	std::string scrubbedErrs;
	std::string scrubbingSince;
	std::string scrubMsg;
	std::string resilverSince;
	std::string resilverMsg;
	int sc = 0;
	std::string msg;
	Pool() {}
	~Pool() {}
	std::string getmsg() {
		if (msg == "") {
			msg = name + ":" + scrubMsg;
		}
		return msg;
	}
	void apply(int _sc, std::string _msg) {
		if (_sc > sc) {
			sc = _sc;
			msg = _msg;
		}
	}
	int status() {
		_assert(name != "");
		_assert(state != "");

		int n = 0;
		if (scrubbedOn != "") n++;
		if (scrubbingSince != "") n++;
		if (resilverSince != "") n++;
		_assert(n == 1);

		_assert(scrubbedOn == "" || (scrubbedRepaired != "" && scrubbedErrs != ""));
		_assert(errors != "");
		_assert(scrubMsg != "");

		if (state != "ONLINE") {
			apply(2, state);
		}
		if (scrubbedOn != "") {
			if (age(scrubbedOn) > MAX_SCRUBBED_AGE_SECS) {
				apply(2, "");
			}
			if (scrubbedRepaired != "0B") {
				apply(1, "");
			}
			if (scrubbedErrs != "0") {
				apply(2, "");
			}
		}
		if (scrubbingSince != "" && age(scrubbingSince) > MAX_SCRUBBING_SINCE_SECS) {
			apply(2, "");
		}
		if (resilverSince != "" && age(resilverSince) > MAX_RESILVERING_SINCE_SECS) {
			apply(2, "");
		}
		if (errors != "No known data errors") {
			apply(2, errors);
		}
		return sc;
	}
};

namespace fs = std::filesystem;

fs::path zpoolExe = ZPOOL_EXE;

int main(int argc, char** argv)
{
	std::vector<std::string> poolNames;
	std::regex cmdRegex("^--(\\S+?)(?:=(.*))?$");
	std::smatch sm;
	for (int i = 1; i < argc; i++) {
		std::string arg(argv[i]);

		if (!std::regex_match(arg, sm, cmdRegex)) {
			if (!arg.empty()) {
				poolNames.push_back(arg);
			}
			else {
				err(3, "invalid command line argument: " + sm[0].str());
			}
		}

		std::string cmd = sm[1];
		std::string cmdArg = sm[2];

		// for testing only:
		if (cmd == "tnow") {
			tnow = (std::time_t)std::stoll(cmdArg);
			continue;
		}
		else if (cmd == "exe") {
			zpoolExe = fs::path(cmdArg).make_preferred();
			_assert(fs::exists(zpoolExe));
			_assert(fs::is_symlink(zpoolExe) || fs::is_regular_file(zpoolExe));
			continue;
		}
	}

	if (poolNames.empty()) {
		err(3, "Please give at least one zpool as argument.");
	}

	std::string s = ssystem(zpoolExe.string() + " status");
	std::istringstream iss(s);
	std::string line;
	std::regex poolRegex("^  pool: (\\S[^\\r\\n]*)[\\r\\n]*$");
	std::regex stateRegex("^ state: (\\S[^\\r\\n]*)[\\r\\n]*$");
	std::regex scanRegex("^  scan: scrub repaired (\\S+) in \\S.*\\S with (\\d+) errors on \\S+ (\\S.*\\S)[\\r\\n]*$");
	std::regex scrubRegex("^  scan: scrub in progress since \\S+ (\\S.*\\S)[\\r\\n]*$");
	std::regex resilverRegex("^  scan: resilver in progress since \\S+ (\\S.*\\S)[\\r\\n]*$");
	std::regex errorsRegex("^errors: (.*)[\\r\\n]*$");
	std::smatch match;
	std::vector<Pool> pools;
	Pool pool;
	while (std::getline(iss, line))
	{
		if (std::regex_match(line, match, poolRegex)) {
			if (pool.name != "") {
				pools.push_back(pool);
				pool = Pool();
			}

			pool.name = match[1];
		}
		// state: ONLINE
		else if (std::regex_match(line, match, stateRegex)) {
			pool.state = match[1];
		}
		//  scan: scrub repaired 0B in 0 days 00:00:11 with 0 errors on Sun Dec 13 00:24:13 2020
		else if (std::regex_match(line, match, scanRegex)) {
			pool.scrubbedRepaired = match[1];
			pool.scrubbedErrs = match[2];
			pool.scrubbedOn = match[3];
			pool.scrubMsg = line.substr(line.find("scan: ") + 6);
		}
		//  scan: scrub in progress since Sun Dec 13 00:24:31 2020
		else if (std::regex_match(line, match, scrubRegex)) {
			pool.scrubbingSince = match[1];
			pool.scrubMsg = line.substr(line.find("scan: ") + 6);
		}
		//  scan: resilver in progress since Sun Dec 13 00:24:31 2020
		else if (std::regex_match(line, match, resilverRegex)) {
			pool.resilverSince = match[1];
			pool.scrubMsg = line.substr(line.find("scan: ") + 6);
		}
		//errors: No known data errors
		else if (std::regex_match(line, match, errorsRegex)) {
			pool.errors = match[1];
		}
	}
	if (pool.name != "") {
		pools.push_back(pool);
	}

	int sc = 0;

	if (sc < 2) {
		for (std::string& name : poolNames) {
			bool found = false;
			for (const Pool& pool : pools) {
				if (pool.name == name) {
					found = true;
				}
			}
			if (!found) {
				sc = std::max(2, sc);
				std::cout << "Pool " << name << " not found in zpool status output." << std::endl;
				break;
			}
		}
	}

	std::string msg;
	for (Pool& pool : pools) {
		sc = std::max(sc, pool.status());
		msg += (!msg.empty() ? " / " : "") + pool.getmsg();
	}

	std::cout << sc << ":" << msg << std::endl;
	exit(sc);
}
