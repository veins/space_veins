# SPDX-FileCopyrightText: 2023 Mario Franke <research@m-franke.net>
#
# SPDX-License-Identifier: GPL-2.0-or-later

from skyfield.api import load, wgs84
import argparse

parser = argparse.ArgumentParser(description='Print satellite overflights at given time and place.')
parser.add_argument('tleFile', metavar='TLE_FILE', help='File which stores the two line element sets.')
parser.add_argument('lat', metavar='LATITUDE', type=float, help='WGS84 latitude of the reference place.')
parser.add_argument('lon', metavar='LONGITUDE', type=float, help='WGS84 longitude of the reference place.')
parser.add_argument('sdate', metavar='START_DATE', help='Start reference date, format: [dd-mm-yyyy].')
parser.add_argument('edate', metavar='END_DATE', help='End reference date, format: [dd-mm-yyyy].')
parser.add_argument('-e', '--min-elevation', dest='min_elevation', type=float, default=70.0, help="Minimum elevation angle the satellite has to reach to be printed [default: 70.0°].")

args = parser.parse_args()

satellites = load.tle_file(args.tleFile)
print('Loaded', len(satellites), 'satellites')

print('Checking epoch:')
print(satellites[0].epoch.utc_jpl())

ts = load.timescale()

location = wgs84.latlon(args.lat, args.lon)

start_date = args.sdate.split('-')
end_date = args.edate.split('-')

if (int(end_date[2]) < int(start_date[2])):
    raise ValueError("End date's year lies before start date!")
else:
    if (int(end_date[1]) < int(start_date[1])):
        raise ValueError("End date's month lies before start date!")
    else:
        if (int(end_date[0]) < int(start_date[0])):
            raise ValueError("End date's day lies before start date!")

t0 = ts.utc(int(start_date[2]), int(start_date[1]), int(start_date[0]))
t1 = ts.utc(int(end_date[2]), int(end_date[1]), int(end_date[0]))

for satellite in satellites:
    t, events = satellite.find_events(location, t0, t1, altitude_degrees=args.min_elevation)
    if len(events) > 0:
        print('Satellite:------------------------------')
        print(satellite)
        for ti, event in zip(t, events):
            name = ('rise above ' + str(args.min_elevation) + '°', 'culminate', 'set below ' + str(args.min_elevation) + '°')[event]
            print(ti.utc_strftime('%Y %b %d %H:%M:%S'), name)
        print('-------------------------')
