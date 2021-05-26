# SPDX-FileCopyrightText: 2021 Mario Franke <research@m-franke.net>
#
# SPDX-License-Identifier: GPL-2.0-or-later

"""
Default configurations for the SkyfieldMobilityServer.
"""

defConfig = {
        'verbosity': 'DEBUG',
        #'tle-file': '../../examples/space_veins/tle_set_25-05-2021-15-24-CEST.txt',
        'tle-file': '../../tle-data/starlink_tle_2021-Aug-16-14:06:06UTC.txt',
        #'simulation-time': '2021-05-25T17:02:35:00_+0000',  # ISS culminates at 17:03:05 ~52deg altitude
        #'simulation-time': '2021-08-16T12:59:04:00_+0000',  # STARLINK-2687 culminates at 13:00:44 ~80deg altitude
        'simulation-time': '2021-08-16T09:00:41:00_+0000',  # STARLINK-1528 culminates at 09:01:11 ~80deg altitude
        'space-veins-port': 40515,
        }
