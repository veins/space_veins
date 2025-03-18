#!/usr/bin/env python3

#
# Copyright (C) 2014-2019 Christoph Sommer <christoph.sommer@uibk.ac.at>
#
# SPDX-License-Identifier: GPL-2.0-or-later
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

#
# Waits until all jobs in a text file are processed, then exits.
#

from __future__ import print_function
import fcntl
import sys
import time
from optparse import OptionParser


class Job:
    """
    Stores a (parsed) line in the job file.
    """

    offset = 0
    length = 0
    state = "."
    cmd = ""

    def __repr__(self):
        return "Job(%s, %s, '%s', '%s')" % (self.offset, self.length, self.state, self.cmd)


def read_jobs(f):
    """
    Read the job file, return the parsed list of jobs.
    """

    jobs = []

    # get a read lock on the whole file
    fcntl.lockf(f, fcntl.LOCK_SH, 0, 0)

    f.seek(0)
    while 1:
        job = Job()
        job.offset = f.tell()
        s = f.readline().decode()
        job.length = f.tell() - job.offset
        if not s:
            break
        if len(s) < 3:
            continue
        if (s[0] == "#" or s[0] == "/"):
            continue
        # line format: <state><one whitespace><commandline>
        if not (s[1] == "\t" or s[1] == " "):
            continue
        s = s.rstrip()
        job.state = s[0]
        job.cmd = s[2:]

        jobs.append(job)

    # release the read lock
    fcntl.lockf(f, fcntl.LOCK_UN, 0, 0)

    return jobs


def refresh_job_states(f, jobs):
    """
    Re-read all job states from the file.
    """

    assert(not f.closed)


    # get a read lock on the whole file
    fcntl.lockf(f, fcntl.LOCK_SH, 0, 0)

    try:
        for job in jobs:
            assert(job.length > 0)
            f.seek(job.offset)
            s = f.read(1).decode()
            job.state = s
    finally:
        # release the read lock
        fcntl.lockf(f, fcntl.LOCK_UN, 0, 0)

    return None


def main():
    """
    Program entry point when run interactively.
    """

    # prepare option parser
    parser = OptionParser(usage="usage: %prog [options] filename", description="Wait until all jobs in a text file are processed.", epilog="In the given file, each line beginning with a dot and a space (. ) will be executed. The file is modified to reflect the execution state of each job (r-running, d-done, !-failed, e-error).")
    parser.add_option("-e", "--use-exit-status", dest="use_exit_status", default=False, action="store_true", help="use exit status 0 only if all jobs are marked done [default: no]")
    parser.add_option("-p", "--progress", dest="progress", default=False, action="store_true", help="show progress while waiting [default: no]")

    # parse options
    (options, args) = parser.parse_args()

    # get file name
    if len(args) < 1:
        print("Need a filename (a list of jobs)")
        print("")
        print(parser.get_usage())
        sys.exit(1)
    fname = args[0]

    # process file

    f = open(fname, 'rb', 0)

    jobs = read_jobs(f)
    states = list()
    while True:
        old_states = states
        refresh_job_states(f, jobs)
        states = list((j.state for j in jobs))

        count_unproc  = len(["." for j in jobs if j.state == '.'])
        count_running = len(["." for j in jobs if j.state == 'r'])
        count_failed  = len(["." for j in jobs if j.state == '!'])
        count_error   = len(["." for j in jobs if j.state == 'e'])
        count_done    = len(["." for j in jobs if j.state == 'd'])

        if states != old_states:
            if options.progress:
                bar_len = 16

                len_running   = int(1.0 * count_running/len(jobs)*bar_len)
                len_failed    = int(1.0 * count_failed /len(jobs)*bar_len)
                len_error     = int(1.0 * count_error  /len(jobs)*bar_len)
                len_done      = int(1.0 * count_done   /len(jobs)*bar_len)

                len_rest = bar_len - (len_running + len_failed + len_error + len_done)
                bar_print = ("=" * len_done) + ("e" * len_error) + ("!" * len_failed) + (">" * len_running) + (" " * len_rest)
                print("progress: %3d of %3d jobs processed, %d errors [%s]" % (count_failed + count_error + count_done, len(jobs), count_failed + count_error, bar_print))

        if count_unproc + count_running == 0:
            if options.use_exit_status and (count_done != len(jobs)):
                sys.exit(1)
            sys.exit(0)

        time.sleep(1)

    f.close()




# Start main() when run interactively
if __name__ == '__main__':
    main()

