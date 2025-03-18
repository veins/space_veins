#!/usr/bin/env python3

#
# Copyright (C) 2012 Christoph Sommer <christoph.sommer@uibk.ac.at>
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
# Reads a text file with jobs and manipulates their state.
#

from __future__ import print_function
import fcntl
import os
import select
import signal
import subprocess
import sys
import multiprocessing
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


def set_job_state(f, job, newstate):
    """
    Do four things:
    - Make sure the job file matches a job object.
    - Modify the job file to reflect a job's new state.
    - Modify a job object to reflect its new state.
    - Return true if successful.
    """

    assert(not f.closed)
    assert(job.length > 0)
    assert(len(newstate) == 1)

    # get an exclusive lock for the byte we will change
    fcntl.lockf(f, fcntl.LOCK_EX, job.offset, 1)

    try:
        f.seek(job.offset)
        s = f.read(1).decode()
        if s != job.state:
            return False
        f.seek(job.offset)
        f.write(newstate.encode())
        f.flush()
    finally:
        # release the exclusive lock
        fcntl.lockf(f, fcntl.LOCK_UN, job.offset, 1)

    job.state = newstate

    return True


def process_file(fname, jobIds, options):
    """
    Manipulate the job file.
    """

    f = open(fname, 'rb+', 0)

    jobs = read_jobs(f)
    for job in jobs:
        if not ((str(job.offset) in jobIds) or (options.all_jobs)):
            continue
        if options.set_state:
            assert(set_job_state(f, job, options.set_state))
        if options.list:
            print("%s: %s - %s" % (job.offset, job.state, job.cmd))

    f.close()


def main():
    """
    Program entry point when run interactively.
    """

    # prepare option parser
    parser = OptionParser(usage="usage: %prog [options] filename jobId jobId ...", description="Read a text file with jobs, manipulate their state.", epilog="In the given file, each line beginning with a dot and a space (. ) will be executed. The file is modified to reflect the execution state of each job (r-running, d-done, !-failed, e-error).")
    parser.add_option("-s", "--set", dest="set_state", default="", help="set state to STATE [default: no change]", metavar="STATE")
    parser.add_option("-l", "--list", dest="list", default=False, action="store_true", help="list given jobs [default: no]")
    parser.add_option("-a", "--all", dest="all_jobs", default=False, action="store_true", help="affect all jobs [default: no]")

    # parse options
    (options, args) = parser.parse_args()

    # get file name
    if len(args) < 1:
        print("Need a filename (a list of jobs)")
        print("")
        print(parser.get_usage())
        sys.exit(1)
    fname = args[0]

    jobIds = []
    if len(args) > 1:
        jobIds = args[1:]

    # process file
    process_file(fname, jobIds, options)


# Start main() when run interactively
if __name__ == '__main__':
    main()

