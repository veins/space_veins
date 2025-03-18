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
# Waits for connections from runmaker4.py asking for a job to perform
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
import tempfile
import logging
import string
import random
from optparse import OptionParser

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

class Command:
    #definition of constants
    #commands
    CMD_UNINITIALIZED = -1
    CMD_GET           = 0
    CMD_SET           = 1
    #errors/results
    VALID_CMD     = 0
    INVALID_CMD   = -1
    INVALID_TOKEN = -2

    #variables
    command = CMD_UNINITIALIZED
    parseResult = INVALID_CMD
    token = ""
    jobNumber = -1
    jobStatus = -1


def read_jobs(f):
    """
    Read the job file, return the parsed list of jobs.
    """

    jobs = []

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

    f.seek(job.offset)
    s = f.read(1).decode()
    if s != job.state:
       return False
    f.seek(job.offset)
    f.write(newstate.encode())
    f.flush()

    job.state = newstate

    return True

def get_new_job(jobs, f, options):
    for job in jobs:
        # keep going until we find a pristine job
        if not ((job.state == '.') or (options.retry and (job.state == '!' or job.state == 'e'))):
            continue
        # try to claim the job
        if not set_job_state(f, job, '?'):
            continue

        return job

    job = Job()
    job.number = -1
    job.cmd = ""
    return job

def parse_command(command, token, options):

    #command to be returned. invalid by default
    cmd = Command()
    #split command in parts
    parts = command.split(" ")

    if (len(parts) == 0):
        return cmd

    if (parts[0] == "GET"):
        #GET format is GET <token>
        if (len(parts) != 2):
            return cmd
        if (parts[1].strip() != token):
            cmd.parseResult = Command.INVALID_TOKEN
            return cmd
        cmd.command = Command.CMD_GET
        cmd.parseResult = Command.VALID_CMD
        return cmd
    elif (parts[0] == "SET"):
        #SET format is SET <token> <job number> <job status>
        if (len(parts) != 4):
            return cmd
        if (parts[1].strip() != token):
            cmd.parseResult = Command.INVALID_TOKEN
            return cmd
        try:
            cmd.jobNumber = int(parts[2])
        except:
            #job number is not a valid integer
            return cmd

        if (not (parts[3] in ['r', 'd', 'e', '!'])):
            return cmd

        cmd.jobStatus = parts[3]
        cmd.command = Command.CMD_SET
        cmd.parseResult = Command.VALID_CMD
        return cmd
    else:
        return cmd


def process_get(jobs, f, client, options, client_address):
    #get a job yet to be done
    job = get_new_job(jobs, f, options)
    logging.debug(str(client_address) + " Returning job number " + str(job.number) + " command: " + job.cmd)
    #return the client the id of the job and the command to execute
    client.sendall((str(job.number) + " " + job.cmd).encode())
    client.recv(2048).decode()

def process_set(jobs, f, client, options, jobn, state, client_address):
    #get all jobs and search for the job requested by the client
    for job in jobs:
        if (job.number == jobn):
            #set the state to the required value
            logging.debug(str(client_address) + " Setting job number " + str(job.number) + " status to " + state)
            set_job_state(f, job, state)
            break


def main():
    """
    Program entry point when run interactively.
    """

    # prepare option parser
    parser = OptionParser(usage="usage: %prog [options] filename", description="Read a text file with jobs, execute them one by one.", epilog="In the given file, each line beginning with a dot and a space (. ) will be executed. The file is modified to reflect the execution state of each job (r-running, d-done, !-failed, e-error).")
    parser.add_option("-r", "--retry", dest="retry", default=False, action="store_true", help="retry failed jobs [default: no]")
    parser.add_option("-l", "--logfile", dest="logfile", default=os.path.join(tempfile.gettempdir(), "runmaker4-server.log"), help="log output to FILENAME [default: %default]", metavar="FILENAME")
    parser.add_option("-v", "--verbose", dest="count_verbose", default=0, action="count", help="increase verbosity [default: don't log infos, debug]")
    parser.add_option("-q", "--quiet", dest="count_quiet", default=0, action="count", help="decrease verbosity [default: log warnings, errors]")
    parser.add_option("-p", "--port", dest="port", type="int", default=9998, action="store", help="TCP PORT the         server has to listen to [default: %default]", metavar="PORT")
    parser.add_option("-d", "--daemon", dest="daemonize", default=False, action="store_true", help="detach and run as daemon [default: no]")
    parser.add_option("-t", "--tokenfile", dest="tokenfile", default=os.path.join(os.path.expanduser("~"), ".runmaker4.token"), action="store", help="string representing the file where the token is stored [default: %default]")

    # parse options
    (options, args) = parser.parse_args()

    _LOGLEVELS = (logging.ERROR, logging.WARN, logging.INFO, logging.DEBUG)
    loglevel = _LOGLEVELS[max(0, min(1 + options.count_verbose - options.count_quiet, len(_LOGLEVELS)-1))]

    # get file name
    if len(args) != 1:
        print("Need exactly one filename (a list of all jobs to run)")
        print("")
        print(parser.get_usage())
        sys.exit(1)
    fname = args[0]

    logging.basicConfig(filename=options.logfile, level=loglevel)
    if not options.daemonize:
        logging.getLogger().addHandler(logging.StreamHandler())
    else:
        print("The --daemon option is not implemented.")
    logging.debug("Logging to %s" % options.logfile)

    f = open(fname, 'rb+', 0)
    jobs = read_jobs(f)

    tokenSize=6
    tokenChars=string.ascii_uppercase + string.digits
    token = ''.join(random.choice(tokenChars) for _ in range(tokenSize))
    print("Token for runmaker4-client.py: %s (written to %s)" % (token, options.tokenfile))
    if (options.tokenfile != ""):
        #we need to write the token to a file
        with os.fdopen(os.open(options.tokenfile, os.O_WRONLY | os.O_CREAT, 0o600), 'w') as handle:
            handle.write(token)

    #create a socket and start listening
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_address = ('0.0.0.0', options.port)
    logging.debug("Starting runmaker4-server.py on port " + str(options.port))
    sock.bind(server_address)

    sock.listen(0)
    run = True
    while run:
        try:
            #accept incoming connection. note that we don't use threads
            #we serve one client at a time, thus automatically synchronizing clients
            client, client_address = sock.accept()

            logging.debug("Connection from " + str(client_address))
            data = client.recv(2048).rstrip().decode()

            cmd = parse_command(data, token, options)

            if cmd.parseResult == Command.INVALID_CMD:
                client.sendall("INVALID_CMD".encode())
                logging.error(str(client_address) +  " Received invalid command: " + data)
            elif cmd.parseResult == Command.INVALID_TOKEN:
                client.sendall("INVALID_TOKEN".encode())
                logging.error(str(client_address) + " Received invalid token. Ignoring request: " + data)
            else:
                if cmd.command == Command.CMD_GET:
                    process_get(jobs, f, client, options, client_address)
                elif cmd.command == Command.CMD_SET:
                    process_set(jobs, f, client, options, cmd.jobNumber, cmd.jobStatus, client_address)
                    client.sendall("ACK".encode())

            client.close()

        except SystemExit:
            run = False
            logging.debug("Killed.")

        except KeyboardInterrupt:
            run = False
            logging.debug("Keyboard interrupt.")

        except:
            raise

    # clean up
    logging.debug("Shutting down.")
    if (options.tokenfile != ""):
        os.remove(options.tokenfile)
    sock.close()

    f.close()

def signal_handler(signal, frame):
    sys.exit(1)

# Start main() when run interactively
if __name__ == '__main__':
    signal.signal(signal.SIGTERM, signal_handler)
    main()

