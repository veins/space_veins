# SPDX-FileCopyrightText: 2023 Mario Franke <research@m-franke.net>
#
# SPDX-License-Identifier: GPL-2.0-or-later

# tested with skyfield 1.46
from skyfield.api import load, wgs84
import argparse

parser = argparse.ArgumentParser(description='Print satellite overflights at given time and place.')
parser.add_argument('tleFile', metavar='TLE_FILE', help='File which stores the two line element sets.')
parser.add_argument('lat', metavar='LATITUDE', type=float, help='WGS84 latitude of the reference place.')
parser.add_argument('lon', metavar='LONGITUDE', type=float, help='WGS84 longitude of the reference place.')
parser.add_argument('sdate', metavar='START_DATE', help='Start reference date, format: [yyyy-mm-dd:hh-mm-ss].')
parser.add_argument('edate', metavar='END_DATE', help='End reference date, format: [yyyy-mm-dd:hh-mm-ss].')
parser.add_argument('-e', '--min-elevation', dest='min_elevation', type=float, default=70.0, help="Minimum elevation angle the satellite has to reach to be printed [default: 70.0°].")

args = parser.parse_args()

satellites = load.tle_file(args.tleFile)
print('Loaded', len(satellites), 'satellites\n')

print('Checking epoch:')
print(satellites[0].epoch.utc_jpl(), "\n")

ts = load.timescale()

location = wgs84.latlon(args.lat, args.lon)

start = args.sdate.split(':')
start_date = start[0]
start_date = start_date.split("-")
start_time = start[1]
start_time = start_time.split("-")

end = args.edate.split(':')
end_date = end[0]
end_date = end_date.split("-")
end_time = end[1]
end_time = end_time.split("-")

if (int(end_date[0]) < int(start_date[0])):
    raise ValueError("End date's year lies before start date!")

if (int(end_date[1]) < int(start_date[1])):
    raise ValueError("End date's month lies before start date!")

if (int(end_date[2]) < int(start_date[2])):
    raise ValueError("End date's day lies before start date!")

if (int(end_time[0]) < int(end_time[0])):
    raise ValueError("End date's hour lies before start date!")

if (int(end_time[1]) < int(end_time[1])):
    raise ValueError("End date's minute lies before start date!")

if (int(end_time[2]) < int(end_time[2])):
    raise ValueError("End date's second lies before start date!")


t0 = ts.utc(int(start_date[0]), int(start_date[1]), int(start_date[2]), int(start_time[0]), int(start_time[1]), int(start_time[2]))
print('start date:')
print(t0.utc_strftime(), "\n")
t1 = ts.utc(int(end_date[0]), int(end_date[1]), int(end_date[2]), int(end_time[0]), int(end_time[1]), int(end_time[2]))
print('end date:')
print(t1.utc_strftime(), "\n")
for satellite in satellites:
    t, events = satellite.find_events(location, t0, t1, altitude_degrees=args.min_elevation)
    if len(events) > 0:
        print('-------------------------------------------------------------')
        print(satellite)
        for ti, event in zip(t, events):
            name = ('rise above ' + str(args.min_elevation) + '°', 'culminate', 'set below ' + str(args.min_elevation) + '°')[event]
            print(ti.utc_strftime('%Y %b %d %H:%M:%S'), name)
        print('-------------------------------------------------------------')