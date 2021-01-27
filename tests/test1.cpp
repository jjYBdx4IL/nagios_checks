#define BOOST_TEST_MODULE Test1
#include <boost/test/unit_test.hpp>

#include "test1.h"

#define BINARY "../checks/check_zpool"
#define BINARY_FREE "../checks/check_zpool_free"
#define BINARY_SMART "../checks/check_smart"
#define STATUS_CURRENT "zpool_output"

#ifdef WIN32
# define ZPOOL_TEST_EXE ".\\zpool"
#else
# define ZPOOL_TEST_EXE "./zpool"
#endif

namespace fs = std::filesystem;

BOOST_AUTO_TEST_SUITE( testsuite1 )

BOOST_AUTO_TEST_CASE(scrubbing_output)
{
	std::string sfn = "zpool_output_scrub";
	BOOST_CHECK_EQUAL(runit(BINARY, sfn, ""), 3);
	BOOST_CHECK_EQUAL(runit(BINARY, sfn, "rpool"), 0);
	BOOST_CHECK_EQUAL(runit(BINARY, sfn, "bla"), 2);
}

BOOST_AUTO_TEST_CASE(resilver_output)
{
	std::string sfn = "zpool_output_resilver";
	BOOST_CHECK_EQUAL(runit(BINARY, sfn, ""), 3);
	BOOST_CHECK_EQUAL(runit(BINARY, sfn, "rpool"), 0);
	BOOST_CHECK_EQUAL(runit(BINARY, sfn, "bla"), 2);
}

BOOST_AUTO_TEST_CASE(resilver_output_long)
{
	std::string sfn = "zpool_output_resilver_long";
	BOOST_CHECK_EQUAL(runit(BINARY, sfn, ""), 3);
	BOOST_CHECK_EQUAL(runit(BINARY, sfn, "rpool"), 2);
	BOOST_CHECK_EQUAL(runit(BINARY, sfn, "bla"), 2);
}

BOOST_AUTO_TEST_CASE(scrubbing_too_long)
{
	std::string sfn = "zpool_output_scrub_long";
	BOOST_CHECK_EQUAL(runit(BINARY, sfn, "rpool"), 2);
}

BOOST_AUTO_TEST_CASE(scrub_too_old)
{
	std::string sfn = "zpool_output_scrub_old";
	BOOST_CHECK_EQUAL(runit(BINARY, sfn, "rpool"), 2);
}

#define TNOW2 " --tnow=1609846681 "
BOOST_AUTO_TEST_CASE(scrubbing_output_bug1)
{
	std::string sfn = "zpool_output_scrub_bug1";
	BOOST_CHECK_EQUAL(runit(BINARY, sfn, "", TNOW2), 3);
	BOOST_CHECK_EQUAL(runit(BINARY, sfn, "rpool", TNOW2), 0);
	BOOST_CHECK_EQUAL(runit(BINARY, sfn, "bla", TNOW2), 2);
}

BOOST_AUTO_TEST_CASE(free_space)
{
	// sudo zpool list -H
	std::string listStr = "bpool   480M    97.7M   382M    -       -       4%      20%     1.00x   ONLINE  -\n"
		"rpool   3.62T   1.04T   2.58T   -       -       0%      28%     1.00x   ONLINE  -\n";
	BOOST_CHECK_EQUAL(runitStr(BINARY_FREE, listStr, "--free=rpool,15%,20%"), 0);
	BOOST_CHECK_EQUAL(runitStr(BINARY_FREE, listStr, "--free=rpool,15%,20% --free=rpool,15%,20%"), 3); //dupe
	BOOST_CHECK_EQUAL(runitStr(BINARY_FREE, listStr, "--free=rpool,15%,20% --free=bpool,5%"), 0);
	BOOST_CHECK_EQUAL(runitStr(BINARY_FREE, listStr, ""), 0);
	BOOST_CHECK_EQUAL(runitStr(BINARY_FREE, listStr, "--wrong"), 3);
	BOOST_CHECK_EQUAL(runitStr(BINARY_FREE, listStr, "--free=bpool,15%,90%"), 1);
	BOOST_CHECK_EQUAL(runitStr(BINARY_FREE, listStr, "--free=bpool,90%,15%"), 1);
	BOOST_CHECK_EQUAL(runitStr(BINARY_FREE, listStr, "--free=bpool,90%,89%"), 2);
	BOOST_CHECK_EQUAL(runitStr(BINARY_FREE, listStr, "--free=bpool,90%"), 2);
	BOOST_CHECK_EQUAL(runitStr(BINARY_FREE, listStr, "--free=bpool,90% --free=rpool,1%"), 2);
	BOOST_CHECK_EQUAL(runitStr(BINARY_FREE, listStr, "--help"), 5);
}

BOOST_AUTO_TEST_CASE(smart)
{
	std::string sfn = "smart_a_output_longok";
	BOOST_CHECK_EQUAL(runit(BINARY_SMART, sfn, "/dev/sda"), 0);
}

BOOST_AUTO_TEST_CASE(smart_nolong)
{
	std::string sfn = "smart_a_output_nolong";
	BOOST_CHECK_EQUAL(runit(BINARY_SMART, sfn, "/dev/sda"), 2);
}

BOOST_AUTO_TEST_SUITE_END()

int runit(std::string binary, std::string statusFn, std::string cmdArg, std::string tnow)
{
	fs::path zpoolExe = fs::path(binary).make_preferred();
	fs::copy_file(fs::path(statusFn), fs::path(STATUS_CURRENT), fs::copy_options::overwrite_existing);
	int sc = std::system((zpoolExe.string() + " --exe=" + ZPOOL_TEST_EXE + " " + tnow + cmdArg).c_str());
#ifdef __unix__
	sc = WEXITSTATUS(sc);
#endif
	return sc;
}

int runitStr(std::string binary, std::string content, std::string cmdArg)
{
	fs::path zpoolExe = fs::path(binary).make_preferred();
	writeFile(fs::path(STATUS_CURRENT), content);
	int sc = std::system((zpoolExe.string() + " --exe=" + ZPOOL_TEST_EXE + " " + TNOW + cmdArg).c_str());
#ifdef __unix__
	sc = WEXITSTATUS(sc);
#endif
	return sc;
}


