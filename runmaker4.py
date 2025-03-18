#!/usr/bin/env python3

#
# Copyright (C) 2012-2019 Christoph Sommer <christoph.sommer@uibk.ac.at>
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
# Reads a text file with jobs, executes them one by one.
# Synchronization between multiple instances (potentially on different machines) is done via the text file.
# The text file must reside on a shared filesystem supporting locking.
#
# Based on a joint idea with David Eckhoff <eckhofff@cs.fau.de>
#

from __future__ import print_function
import fcntl
import os
import select
import signal
import subprocess
import sys
import multiprocessing
import time
from optparse import OptionParser

# log file is this many characters wide
LOGWIDTH = 160

# number of seconds between each logging of output to log file
LOGMAXDELAY = 60

class Job:
    """
    Stores a (parsed) line in the job file.
    """

    number = 0
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
        job.number = len(jobs)+1
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


def run_job(job, options):
    """
    Fork and execute the job, wait for completion, return the exit code.
    """

    s = "executing `%s'" % job.cmd
    print(s)

    logf = None
    log = [":".ljust(LOGWIDTH) for i in range(options.logfile_lines)]
    log_changed = True
    last_log_write = 0
    if options.logfile:
        s = ".-> %s (in %s)" % (job.cmd, os.getcwd())
        s = "%s\n" % s[:LOGWIDTH].ljust(LOGWIDTH)
        logf = open(options.logfile, 'rb+', 0)
        logf.seek((job.number - 1) * (LOGWIDTH + 1) * (options.logfile_lines + 1))
        logf.write(s.encode())
        for s in log:
            logf.write(("%s\n" % s).encode())

    opp = subprocess.Popen(job.cmd, shell=True, preexec_fn=os.setpgrp, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, )
    try:
        opp_pid = "%s,%s" % (os.uname()[1], opp.pid)
        s = "status (%s): %s \"%s\"" % (opp_pid, "forked", job.cmd)
        print(s)
        if logf:
            s = "+ %s" % s
            log.pop(0)
            log.append(s[:LOGWIDTH].ljust(LOGWIDTH))
        opp.stdin.close()

        poll = select.poll()
        poll.register(opp.stdout, select.POLLIN | select.POLLHUP)
        poll.register(opp.stderr, select.POLLIN | select.POLLHUP)
        pollc = 2

        next_wakeup = max(0.001, last_log_write + LOGMAXDELAY - time.time())*1000
        events = poll.poll(next_wakeup)
        while pollc > 0:
            for event in events:
                (rfd, event) = event
                if event & select.POLLIN:
                    if rfd == opp.stdout.fileno():
                        line = opp.stdout.readline().decode()
                        if len(line) > 0:
                            s = "stdout (%s): %s" % (opp_pid, line[:-1])
                            if logf:
                                s = ": %s" % s
                                log.pop(0)
                                log.append(s[:LOGWIDTH].ljust(LOGWIDTH))
                                log_changed = True
                            else:
                                print(s)
                    if rfd == opp.stderr.fileno():
                        line = opp.stderr.readline().decode()
                        if len(line) > 0:
                            s = "stderr (%s): %s" % (opp_pid, line[:-1])
                            if logf:
                                s = "! %s" % s
                                log.pop(0)
                                log.append(s[:LOGWIDTH].ljust(LOGWIDTH))
                                log_changed = True
                            else:
                                print(s)
                if event & select.POLLHUP:
                    poll.unregister(rfd)
                    pollc = pollc - 1
            if logf:
                if log_changed and (time.time() - last_log_write) >= LOGMAXDELAY:
                    logf.seek((job.number - 1) * (LOGWIDTH + 1) * (options.logfile_lines + 1) + (LOGWIDTH + 1))
                    for s in log:
                        logf.write(("%s\n" % s).encode())
                    last_log_write = time.time()
                    log_changed = False
            if pollc > 0:
                next_wakeup = max(0.001, last_log_write + LOGMAXDELAY - time.time())*1000
                events = poll.poll(next_wakeup)
        returncode = opp.wait()
        s = "status (%s): %s %s \"%s\"" % (opp_pid, "exit", returncode, job.cmd)
        print(s)
        if logf:
            s = "+ %s" % s
            log.pop(0)
            log.append(s[:LOGWIDTH].ljust(LOGWIDTH))
        if logf:
            logf.seek((job.number - 1) * (LOGWIDTH + 1) * (options.logfile_lines + 1) + (LOGWIDTH + 1))
            for s in log:
                logf.write(("%s\n" % s).encode())
        return returncode

    except:
        os.killpg(os.getpgid(opp.pid), signal.SIGINT)
        raise

    finally:
        if logf:
            logf.close()



def process_file(fname, options):
    """
    Open the job file, and for each job to be executed, execute it.
    """

    f = open(fname, 'rb+', 0)

    jobs = read_jobs(f)
    for job in jobs:
        # keep going until we find a pristine job
        if not ((job.state == '.') or (options.retry and (job.state == '!' or job.state == 'e'))):
            continue
        # try to claim the job
        if not set_job_state(f, job, '?'):
            continue
        # from here on out, the job is ours
        try:
            assert(set_job_state(f, job, 'r'))
            if run_job(job, options) == 0:
                assert(set_job_state(f, job, 'd'))
                if options.one_only:
                    break
            else:
                assert(set_job_state(f, job, '!'))
        except:
            #print "Error while executing:", sys.exc_info()[0]
            assert(set_job_state(f, job, 'e'))
            raise

    f.close()



def main():
    """
    Program entry point when run interactively.
    """

    # prepare option parser
    parser = OptionParser(usage="usage: %prog [options] filename", description="Read a text file with jobs, execute them one by one.", epilog="In the given file, each line beginning with a dot and a space (. ) will be executed. The file is modified to reflect the execution state of each job (r-running, d-done, !-failed, e-error).")
    parser.add_option("-j", "--jobs", dest="num_jobs", type="int", default=1, action="store", help="start NUMBER jobs in parallel, 0 meaning autodetect [default: %default]", metavar="NUMBER")
    parser.add_option("-r", "--retry", dest="retry", default=False, action="store_true", help="retry failed jobs [default: no]")
    parser.add_option("-l", "--logfile", dest="logfile", default="", help="log output to FILENAME [default: none]", metavar="FILENAME")
    parser.add_option("-n", "--loglines", dest="logfile_lines", type="int", default=3, action="store", help="if logging, log the last NUMBER lines of output [default: %default]", metavar="NUMBER")
    parser.add_option("-1", "--one-only", dest="one_only", default=False, action="store_true", help="run no more than a single job before exiting [default: no]")

    # parse options
    (options, args) = parser.parse_args()

    # get file name
    if len(args) != 1:
        print("Need exactly one filename (a list of all jobs to run)")
        print("")
        print(parser.get_usage())
        sys.exit(1)
    fname = args[0]

    # autodetect number of cpus
    if options.num_jobs == 0:
        try:
            options.num_jobs = multiprocessing.cpu_count()
        except:
            pass

    # spawn children
    children = []
    for i in range(options.num_jobs):
        child = multiprocessing.Process(target=process_file, args=(fname,options,))
        child.start()
        children.append(child)
    for child in children:
        child.join()


# Start main() when run interactively
if __name__ == '__main__':
    main()

