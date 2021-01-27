# nagios_checks

#### Nagios checks for: ZFS scrub/space, SMART self-test

Note: it's recommended to use "export TZ=UTC LC_ALL=C" in any
shell scripts before calling one of these tools.

### check_zpool

Beta version. Use at your own risk.

It currently does some sanity checks agaimst "zpool status"
console output. Plus it sets a limit of 10 days for the last
scrub of each pool and a limit of 3 days for ongoing scrubs.

For details see `check_zpool.cpp`.

    Usage:
    
    check_zpool pool#1 [pool#2] [...]

It's checking all pools it can find in zpool status output,
but specifying the pools has the added benefit of making
sure that it actually got parsed/found in the output.

Exit code is nagios status code, ie. 0 for OK, 1 for WARNING,
2 for ERROR. Regular output is of the form:

    <exit code>:<message>

Should build on Linux and Windows (MSVC).

It's totally possible that this tool is only able to parse
zpool status output when locale is set to en_US.UTF-8.

As of now, there are only two unofficial test options 
suppliable to the check_zpool command (used for testing):
--tnow=%d and --exe=%s. It may be possible to use the second
option to run check_zpool(_free) as a restricted user and
run zpool via sudo.

If you have non-mirrored volumes, you'd probably have to add some
parameter(s) to allow for that situation.

### check_zpool_free

Check your free pool capacity (similar to check_disk).

    dev@nas:~$ /usr/local/bin/check_zpool_free --help
    usage: check_zpool_free [--free=<POOL>,10%[,20%]] [...]
    By default, warning/critical free capacity values are 10% and 5% for each device
    found in 'zpool list' output. The lower of both values is always the critical one.
    You can set default values using the empty pool name.

### check_smart

    dev@nas:~$ /usr/local/bin/check_smart --help
    usage: check_smart /dev/sda [...]
    Checks SMART self-test log if last successful short (long) selftest is not older than 3 (10) days.
    Does *NOT* check for failed tests.



--
devel/cpp/nagios_checks@7776
