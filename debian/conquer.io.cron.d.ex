#
# Regular cron jobs for the conquer.io package.
#
0 4	* * *	root	[ -x /usr/bin/conquer.io_maintenance ] && /usr/bin/conquer.io_maintenance
