import csv
import numpy as np
from skyfield.api import load
import argparse
import datetime

parser = argparse.ArgumentParser(description='Compute UT1 times.')
parser.add_argument('sdate', metavar='START_DATE', help='UTC start date, format: [yyyy-mm-dd:hh-mm-ss].')
parser.add_argument('duration', metavar='DURATION', type=float, help='Duration of the simulation in seconds.')
parser.add_argument('leoMobilityUpdateInterval', metavar='INTERVAL', type=float, help='Time interval between two periodic LEO satellite mobility updates in seconds.')

if __name__ == "__main__":
    args = parser.parse_args()
    # start
    # date
    start = args.sdate.split(':')
    start_date = start[0]
    start_date = start_date.split("-")
    # time
    start_time = start[1]
    start_time = start_time.split("-")
    # datetime
    datetimeStartDate = datetime.datetime(int(start_date[0]), int(start_date[1]), int(start_date[2]), int(start_time[0]), int(start_time[1]), int(start_time[2]))
    seconds_offset = np.arange(float(start_time[2]), float(start_time[2]) + args.duration + args.leoMobilityUpdateInterval, args.leoMobilityUpdateInterval)
    print(seconds_offset)
    # Load the standard Skyfield timescale
    ts = load.timescale()
    t = ts.utc(int(start_date[0]), int(start_date[1]), int(start_date[2]), int(start_time[0]), int(start_time[1]), seconds_offset)
    utc_times = t.utc_strftime('%Y-%m-%d %H:%M:%S.%f')

    csv_filename = "ut1.csv"
    print(f"Writing data to {csv_filename}...")

    # Write the data to a CSV file
    with open(csv_filename, mode='w', newline='') as csv_file:
        writer = csv.writer(csv_file)

        writer.writerow(["time_utc", "time_offset_s", "julian_date_day", "julian_date_frac_ut1"])

        for i in range(len(seconds_offset)):
            writer.writerow([
                utc_times[i],
                f"{i * args.leoMobilityUpdateInterval:.2f}",
                f"{t[i].whole:.1f}",
                f"{t[i].ut1_fraction:.17f}",
            ])
