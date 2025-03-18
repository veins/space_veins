#!/usr/bin/env python3

#
# Copyright (C) 2012-2019 Christoph Sommer <christoph.sommer@uibk.ac.at>
# Copyright (C) 2015 Michele Segata <segata@ccs-labs.org>
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
# Executes jobs provided via network by runmaker4-server.py
#

from __future__ import print_function
import fcntl
import os
import select
import signal
import subprocess
import sys
import socket
import multiprocessing
import random
import time
from optparse import OptionParser

LOGWIDTH = 500

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


def set_job_state(job, newstate, host, options):

    #do 5 attempts
    attempts = 5
    while (attempts > 0):
        attempts = attempts - 1
        try:
            #connect to the server
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sa = (host, options.port)
            sock.connect(sa)
            #send command to change job status
            sock.sendall(("SET " + options.token + " " + str(job.number) + " " + newstate).encode())
            #receive the ack, to be sure server go the message
            sock.recv(2048).decode()
            #close the connection
            sock.close()
            return
        except:
            #if something went wrong, wait a little and try again
            print("Error connecting to server. Retrying in a few seconds.")
            time.sleep(random.uniform(0,3))
            continue

    raise


def run_job(job, options):
    """
    Fork and execute the job, wait for completion, return the exit code.
    """

    s = "executing `%s'" % job.cmd
    print(s)

    logf = None
    log = [":".ljust(LOGWIDTH) for i in range(options.logfile_lines)]
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

        events = poll.poll()
        while pollc > 0 and len(events) > 0:
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
                            else:
                                print(s)
                if event & select.POLLHUP:
                    poll.unregister(rfd)
                    pollc = pollc - 1
                if logf:
                    logf.seek((job.number - 1) * (LOGWIDTH + 1) * (options.logfile_lines + 1) + (LOGWIDTH + 1))
                    for s in log:
                        logf.write(("%s\n" % s).encode())
                if pollc > 0:
                    events = poll.poll()
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

def process_file(host, options):
    """
    Open the job file, and for each job to be executed, execute it.
    """
    run = True
    lastException = 0
    while run:
        #do 5 attempts for each request, to be sure to avoid problems due to
        #concurrent with the others
        attempts = 5
        while (attempts > 0):
            attempts = attempts - 1
            job_done = False
            try:
                #connect to server
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sa = (host, options.port)
                sock.connect(sa)

                #ask for a job
                sock.sendall(("GET " + options.token).encode())
                response = sock.recv(2048).decode()
                if (response == ""):
                    print("Empty server response")
                    time.sleep(random.uniform(0,3))
                    continue
                if (response == "INVALID_CMD"):
                    print("Got invalid command error from server. Check the code. Quitting")
                    sys.exit(1)
                if (response == "INVALID_TOKEN"):
                    print("Got invalid token error. Check that token or token file are correct. Quitting")
                    sys.exit(1)

                sock.sendall("ACK".encode())
                #job looks like "JOBID CMD"
                v = response.split(" ", 1)

                job = Job()
                job.number = int(v[0])
                if (job.number != -1):
                    job.cmd = v[1]

                    #run the job
                    try:
                        set_job_state(job, 'r', host, options)
                        if run_job(job, options) == 0:
                            job_done = True
                            set_job_state(job, 'd', host, options)
                        else:
                            set_job_state(job, '!', host, options)
                    except KeyboardInterrupt as ki:
                        #if the user hits ctrl-c, set job status to e, and exit
                        set_job_state(job, 'e', host, options)
                        return False

                else:
                    #server said there's nothing left to do. stop here
                    return True

            except Exception as ex:
                #if we got an exception, sleep for a random amount of time
                #then ask again
                print("Exception caught. Retrying in a few seconds.")
                lastException = ex
                time.sleep(random.uniform(0,3))
                continue

        #when the number of attempts goes to 0, then something bad is going on. stop everything
        if (not job_done and attempts == 0):
            print("Job quitting because of consecutive errors.")
            if (lastException != 0):
                print("Last exception:")
                print(lastException)
            run = False


def main():
    """
    Program entry point when run interactively.
    """

    # prepare option parser
    parser = OptionParser(usage="usage: %prog [options] host", description="Run", epilog="Refer to the help output of runmaker4-server.py for more details.")
    parser.add_option("-j", "--jobs", dest="num_jobs", type="int", default=1, action="store", help="start NUMBER jobs in parallel, 0 meaning autodetect [default: %default]", metavar="NUMBER")
    parser.add_option("-l", "--logfile", dest="logfile", default="", help="log output to FILENAME [default: none]", metavar="FILENAME")
    parser.add_option("-n", "--loglines", dest="logfile_lines", type="int", default=3, action="store", help="if logging, log the last NUMBER lines of output [default: %default]", metavar="NUMBER")
    parser.add_option("-p", "--port", dest="port", type="int", default=9998, action="store", help="TCP PORT the server is listening to [default: %default]", metavar="PORT")
    parser.add_option("-t", "--token", dest="token", default=os.path.join(os.path.expanduser("~"), ".runmaker4.token"), action="store", help="string representing either the token or the file where the token is stored. The token is sent to the server at each request for authentication purpose. If the parameter ends with .token, it is assumed that the token needs to be read from a file. [default: %default]")

    # parse options
    (options, args) = parser.parse_args()

    if options.logfile:
        if (not os.path.exists(options.logfile)) or (not os.path.isfile(options.logfile)):
            print("You need to create log file %s before starting runmaker4-client. Stop" % options.logfile)
            sys.exit(1)

    # get file name
    if len(args) != 1:
        print("Need host address or name")
        print("")
        print(parser.get_usage())
        sys.exit(1)
    host = args[0]

    if (options.token.endswith(".token")):
        #we need to take the token from a file
        try:
            tokenFile = open(options.token, 'r')
            #overwrite the filename with the token. we'll pass this to the subprocesses
            options.token = tokenFile.read()
            tokenFile.close()
        except:
            print ("Error occured while retrieving the token. Does the token file exist?")
            sys.exit(1)

    # autodetect number of cpus
    if options.num_jobs == 0:
        try:
            options.num_jobs = multiprocessing.cpu_count()
        except:
            pass

    # spawn children
    children = []
    for i in range(options.num_jobs):
        child = multiprocessing.Process(target=process_file, args=(host,options))
        child.start()
        children.append(child)
    for child in children:
        child.join()

# Start main() when run interactively
if __name__ == '__main__':
    main()

