#include "check_smart.h"

#define MAX_LONGTEST_AGE_HOURS (10L * 24)
#define MAX_SHORTTEST_AGE_HOURS (3L * 24)

#define SMARTCTL_EXE "smartctl"

namespace fs = std::filesystem;

fs::path smartctlExe = SMARTCTL_EXE;

std::vector<fs::path> disks;

std::ostringstream ss;

std::string getSection(const std::string& input, const std::string start) {
	std::size_t pos = input.find(start);
	if (pos == input.npos) {
		return "";
	}
	std::size_t endpos = input.find("\n\n", pos + start.length());
	if (endpos == input.npos) {
		return input.substr(pos);
	}
	return input.substr(pos, endpos - pos);
}

int main(int argc, char** argv)
{
	//
	// parse command line
	//

	std::regex cmdRegex("^--(\\S+?)(?:=(.*))?$");
	std::smatch m;
	for (int i = 1; i < argc; i++) {
		std::string arg(argv[i]);

		if (!std::regex_match(arg, m, cmdRegex)) {
			if (arg.empty())
				err(3, "invalid command line argument: " + arg);
			fs::path dev(arg);
			disks.push_back(dev);
			continue;
		}

		std::string cmd = m[1].str();
		std::string cmdArg = m[2].str();

		// for testing only:
		if (cmd == "exe") {
			smartctlExe = fs::path(cmdArg).make_preferred();
			_assert(fs::exists(smartctlExe));
			_assert(fs::is_symlink(smartctlExe) || fs::is_regular_file(smartctlExe));
			continue;
		}
		else if (cmd == "tnow") {
			//ignore
		}
		else if (cmd == "help") {
			std::cout
				<< "usage: check_smart /dev/sda [...]" << std::endl
				<< "Checks SMART self-test log if last successful short (long) selftest is not older than 3 (10) days." << std::endl
				<< "Does *NOT* check for failed tests." << std::endl;
			exit(5);
		}
		else {
			err(3, "invalid command: " + arg);
		}
	}

	if (disks.empty()) {
		err(3, "no devices given");
	}

	int sc = 0;
	std::vector<std::string> msgs;

	//
	// parse "smartctl -a" output
	//

	for (const fs::path& dev : disks) {
		std::string s = ssystem(smartctlExe.string() + " -a " + dev.string());

		// get the device serial
		std::string infoSectionStr = getSection(s, "\n\n=== START OF INFORMATION SECTION ===");

		std::regex devSerialRegex("\\nSerial Number:\\s+(\\S.*)");
		if (!std::regex_search(infoSectionStr, m, devSerialRegex))
			err(3, "cannot find device serial for " + dev.string());

		const std::string devSerial = m[1].str();

		// extract the device's internal time reference (power on hours) ...

		//<empty line>
		//SMART Attributes Data Structure revision number : 16
		//  Vendor Specific SMART Attributes with Thresholds :
		//ID# ATTRIBUTE_NAME          FLAG     VALUE WORST THRESH TYPE      UPDATED  WHEN_FAILED RAW_VALUE
		//  1 Raw_Read_Error_Rate     0x002f   200   200   051    Pre - fail  Always - 0
		//...
		//  9 Power_On_Hours          0x0032   100   100   000    Old_age   Always - 10
		//...
		//<empty line>
		std::string attrsSectionStr = getSection(s, "\n\nSMART Attributes Data Structure revision number");
		if (attrsSectionStr.empty())
			err(3, "cannot find SMART Attributes section for " + devSerial);
		
		std::regex pwrOnHrsRegex("\\n\\s*9\\s+Power_On_Hours\\s+.*?\\s+(\\d+)\\n");
		if (!std::regex_search(attrsSectionStr, m, pwrOnHrsRegex))
			err(3, "cannot find Power_On_Hours SMART Attribute for " + devSerial);

		long pwrOnHrs = std::stol(m[1].str());

		// ... and compare it against last successful completions of both short and long self-tests:

		//<empty line>
		//SMART Self-test log structure revision number 1
		//Num  Test_Description    Status                  Remaining  LifeTime(hours)  LBA_of_first_error
		//# 1  Short offline       Completed without error       00 % 0 -
		//<empty line>
		std::string selfTestLogStr = getSection(s, "\n\nSMART Self-test log structure");
		if (selfTestLogStr.empty())
			err(3, "cannot find Self-test log structure output for " + devSerial);

		std::regex shortTestRegex("\\n#\\s*\\d+\\s+Short offline\\s+Completed without error\\s+00\\%\\s+(\\d+)\\s+-");
		std::regex longTestRegex("\\n#\\s*\\d+\\s+Extended offline\\s+Completed without error\\s+00\\%\\s+(\\d+)\\s+-");

		if (!std::regex_search(selfTestLogStr, m, shortTestRegex)) {
			sc = std::max(2, sc);
			msgs.push_back("no successful short test execution found for " + devSerial);
			continue;
		}

		long shortTestAgeHrs = pwrOnHrs - std::stol(m[1].str());

		if (shortTestAgeHrs > MAX_SHORTTEST_AGE_HOURS) {
			sc = std::max(2, sc);
			ss.str("");
			ss << "last successful short test execution too old (" << shortTestAgeHrs << " hrs): " << devSerial;
			msgs.push_back(ss.str());
			continue;
		}

		if (!std::regex_search(selfTestLogStr, m, longTestRegex)) {
			sc = std::max(2, sc);
			msgs.push_back("no successful long test execution found for " + devSerial);
			continue;
		}

		long longTestAgeHrs = pwrOnHrs - std::stol(m[1].str());

		if (longTestAgeHrs > MAX_LONGTEST_AGE_HOURS) {
			sc = std::max(2, sc);
			ss.str("");
			ss << "last successful long test execution too old (" << longTestAgeHrs << " hrs): " << devSerial;
			msgs.push_back(ss.str());
			continue;
		}

		ss.str("");
		ss << "OK (" << devSerial << " test success age - short " << shortTestAgeHrs
			<< "h, long " << longTestAgeHrs << "h)";
		msgs.push_back(ss.str());
	}

	err(sc, boost::algorithm::join(msgs, " / "));
}
