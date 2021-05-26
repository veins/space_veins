#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2021 Mario Franke <research@m-franke.net>
#
# SPDX-License-Identifier: GPL-2.0-or-later

"""
A server which provides mobility data of satellites for space veins.
"""

import skyfieldSessionManager
import argparse
import logging
from skyfieldConfigDefaults import defConfig
# import skyfieldProtobufInterface
# import skyfieldSatelliteMobilityManager

log = logging.getLogger(__name__)

def build_arg_parser():
    parser = argparse.ArgumentParser(
            description = 'SkyfieldMobilityServer - it provides the satellite mobility for Space Veins based on Two-line elements.'
            )
    parser.add_argument(
            "-v",
            "--verbosity",
            choices = ['DEBUG', 'INFO', 'WARNING', 'ERROR'],
            default = defConfig['verbosity'],
            help = "Set log level [default: {}].".format(defConfig['verbosity'])
            )
    parser.add_argument(
            "-t",
            "--tle-file",
            default = defConfig['tle-file'],
            help = "Specify two-line elements file [default: {}].".format(defConfig['tle-file'])
            )
    parser.add_argument(
            "-s",
            "--simulation-time",
            default = defConfig['simulation-time'],
            help = "Specify the simulated time [default: {}].".format(defConfig['simulation-time'])
            )
    parser.add_argument(
            "-p",
            "--space-veins-port",
            default = defConfig['space-veins-port'],
            help = "Specify the port on which the SkyfieldMobilityServer listens to [default: {}].".format(defConfig['space-veins-port'])
            )
    return parser

def init_logging():
    logging.basicConfig(level=logging.DEBUG)

def main():
    """
    Run the SkyfieldMobilityServer.
    """

    init_logging()

    log.info("Start SkyfieldMobilityServer")
    #parse commandline arguments
    args = build_arg_parser().parse_args()
    ssm = skyfieldSessionManager.SkyfieldSessionManager(args.tle_file, args.simulation_time, args.space_veins_port)
    ssm.run()
    log.info('Exit SkyfieldMobilityServer')

if __name__ == '__main__':
    main()
